"""Synthetic checks for the four-input arithmetic and input validation."""

import json
import tempfile
import unittest
from pathlib import Path

from metrics import compare, read_summary


def summary(source_id, events, sdc, delivered):
    return {
        "schema_version": 1,
        "cells": [
            {
                "source_id": source_id,
                "input_id": f"input-{index + 1}",
                "target_index": None,
                "sampling_mode": "random_dynamic_event",
                "fault_scope": "whole_program",
                "n": delivered[index],
                "attempted": delivered[index],
                "counts": {
                    "sdc": sdc[index],
                    "correct_masked": delivered[index] - sdc[index],
                },
                "all_deliveries_classified": True,
                "dynamic_events": events[index],
                "source_sha256": source_id * 64,
                "ir_sha256": source_id.upper() * 64,
            }
            for index in range(len(events))
        ],
    }


class MetricsTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)

    def read(self, name, payload):
        path = Path(self.temp.name) / name
        path.write_text(json.dumps(payload), encoding="utf-8")
        return read_summary(path)

    def test_equal_input_rates_and_event_normalization(self):
        reference = self.read("reference.json", summary(
            "r", [10, 20, 30, 40], [1, 2, 3, 4], [10, 10, 10, 10]))
        candidate = self.read("candidate.json", summary(
            "c", [20, 10, 60, 40], [2, 0, 4, 2], [10, 20, 10, 10]))
        result = compare(reference, candidate, "t1", "synthetic")
        self.assertAlmostEqual(result["equal_input_mean_rates"]["reference"]["sdc"], 0.25)
        self.assertAlmostEqual(result["equal_input_mean_rates"]["candidate"]["sdc"], 0.20)
        self.assertAlmostEqual(result["normalized_exposure"]["reference"], 0.25)
        self.assertAlmostEqual(result["normalized_exposure"]["candidate"], 0.35)
        self.assertAlmostEqual(result["normalized_exposure"]["candidate_minus_reference"], 0.10)

    def test_rejects_incomplete_input_set(self):
        incomplete = summary("r", [10, 20, 30], [1, 2, 3], [10, 10, 10])
        with self.assertRaisesRegex(ValueError, "exactly 4"):
            self.read("incomplete.json", incomplete)

    def test_rejects_targeted_fi_and_unclassified_delivery(self):
        targeted = summary("r", [10] * 4, [1] * 4, [10] * 4)
        targeted["cells"][0]["sampling_mode"] = "per_instruction"
        with self.assertRaisesRegex(ValueError, "whole-program random FI"):
            self.read("targeted.json", targeted)
        incomplete = summary("r", [10] * 4, [1] * 4, [10] * 4)
        incomplete["cells"][0]["all_deliveries_classified"] = False
        with self.assertRaisesRegex(ValueError, "not all classified"):
            self.read("unclassified.json", incomplete)

    def test_rejects_mismatched_input_ids(self):
        reference = self.read("reference.json", summary(
            "r", [10] * 4, [1] * 4, [10] * 4))
        changed = summary("c", [10] * 4, [1] * 4, [10] * 4)
        changed["cells"][0]["input_id"] = "different-input"
        candidate = self.read("candidate.json", changed)
        with self.assertRaisesRegex(ValueError, "same four input IDs"):
            compare(reference, candidate, "t3", "synthetic")


if __name__ == "__main__":
    unittest.main()
