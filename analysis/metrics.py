#!/usr/bin/env python3
"""Compute basic T1/T3 FI outcome rates and normalized exposure.

Inputs are two JSON summaries produced by fi/summarize.py from separate,
whole-program random-FI campaigns. No study observations are bundled here.
"""

import argparse
import json
import sys
from pathlib import Path


OUTCOMES = ("correct_masked", "sdc", "detected_failure", "crash", "timeout")
INPUT_COUNT = 4


def _integer(value, name, minimum=0):
    if type(value) is not int or value < minimum:
        raise ValueError(f"{name} must be an integer >= {minimum}")
    return value


def read_summary(path, requested_source_id=None):
    """Validate one fi/summarize.py export and return its four random-FI cells."""
    document = json.loads(Path(path).read_text(encoding="utf-8"))
    if not isinstance(document, dict) or document.get("schema_version") != 1:
        raise ValueError(f"{path}: expected schema_version 1 summary object")
    cells = document.get("cells")
    if not isinstance(cells, list) or not cells:
        raise ValueError(f"{path}: expected a nonempty cells list")
    source_ids = {cell.get("source_id") for cell in cells if isinstance(cell, dict)}
    if requested_source_id is None:
        if len(source_ids) != 1:
            raise ValueError(f"{path}: multiple source IDs; select one explicitly")
        requested_source_id = next(iter(source_ids))
    if not isinstance(requested_source_id, str) or not requested_source_id:
        raise ValueError(f"{path}: source ID must be a nonempty string")

    selected = [cell for cell in cells if isinstance(cell, dict)
                and cell.get("source_id") == requested_source_id]
    if not selected:
        raise ValueError(f"{path}: source ID {requested_source_id!r} not found")
    by_input = {}
    for cell in selected:
        input_id = cell.get("input_id")
        if not isinstance(input_id, str) or not input_id:
            raise ValueError(f"{path}: each input ID must be a nonempty string")
        if input_id in by_input:
            raise ValueError(f"{path}: duplicate input {input_id!r}; use a random-only summary")
        if (cell.get("sampling_mode") != "random_dynamic_event"
                or cell.get("fault_scope") != "whole_program"
                or cell.get("target_index") is not None):
            raise ValueError(f"{path}: {input_id}: only whole-program random FI is supported")
        if cell.get("all_deliveries_classified") is not True:
            raise ValueError(f"{path}: {input_id}: delivered faults are not all classified")
        delivered = _integer(cell.get("n"), f"{path}: {input_id}: n", 1)
        attempted = _integer(cell.get("attempted"), f"{path}: {input_id}: attempted", delivered)
        events = _integer(cell.get("dynamic_events"),
                          f"{path}: {input_id}: dynamic_events", 1)
        counts = cell.get("counts")
        if not isinstance(counts, dict) or set(counts) - set(OUTCOMES):
            raise ValueError(f"{path}: {input_id}: unexpected or invalid outcome counts")
        validated = {name: _integer(counts.get(name, 0),
                                    f"{path}: {input_id}: {name}") for name in OUTCOMES}
        if sum(validated.values()) != delivered:
            raise ValueError(f"{path}: {input_id}: outcome counts do not sum to n")
        by_input[input_id] = {
            "n": delivered, "attempted": attempted,
            "dynamic_events": events, "counts": validated,
            "source_sha256": cell.get("source_sha256"),
            "ir_sha256": cell.get("ir_sha256"),
        }
    if len(by_input) != INPUT_COUNT:
        raise ValueError(f"{path}: expected exactly {INPUT_COUNT} distinct inputs, got {len(by_input)}")
    source_hashes = {cell["source_sha256"] for cell in by_input.values()}
    ir_hashes = {cell["ir_sha256"] for cell in by_input.values()}
    if (len(source_hashes) != 1 or len(ir_hashes) != 1
            or not all(isinstance(value, str) and value for value in source_hashes | ir_hashes)):
        raise ValueError(f"{path}: source/IR identities differ across inputs or are missing")
    return {
        "source_id": requested_source_id,
        "source_sha256": next(iter(source_hashes)),
        "ir_sha256": next(iter(ir_hashes)),
        "inputs": by_input,
    }


def compare(reference, candidate, mode, pair_id):
    """Use equal input weights and Eq. (1), without inferential statistics."""
    if mode not in ("t1", "t3"):
        raise ValueError("mode must be t1 or t3")
    if not isinstance(pair_id, str) or not pair_id:
        raise ValueError("pair ID must be a nonempty string")
    ref_inputs, cand_inputs = reference["inputs"], candidate["inputs"]
    if len(ref_inputs) != INPUT_COUNT or set(ref_inputs) != set(cand_inputs):
        raise ValueError("reference and candidate must have the same four input IDs")

    rows = []
    sums = {role: {name: 0.0 for name in OUTCOMES}
            for role in ("reference", "candidate")}
    ref_exposure = cand_exposure = 0.0
    for input_id in sorted(ref_inputs):
        ref, cand = ref_inputs[input_id], cand_inputs[input_id]
        ref_rates = {name: ref["counts"][name] / ref["n"] for name in OUTCOMES}
        cand_rates = {name: cand["counts"][name] / cand["n"] for name in OUTCOMES}
        for name in OUTCOMES:
            sums["reference"][name] += ref_rates[name] / INPUT_COUNT
            sums["candidate"][name] += cand_rates[name] / INPUT_COUNT
        event_ratio = cand["dynamic_events"] / ref["dynamic_events"]
        ref_exposure += ref_rates["sdc"] / INPUT_COUNT
        cand_exposure += event_ratio * cand_rates["sdc"] / INPUT_COUNT
        rows.append({
            "input_id": input_id,
            "reference": {"delivered": ref["n"], "attempted": ref["attempted"],
                          "clean_eligible_events": ref["dynamic_events"], "rates": ref_rates},
            "candidate": {"delivered": cand["n"], "attempted": cand["attempted"],
                          "clean_eligible_events": cand["dynamic_events"], "rates": cand_rates},
            "candidate_to_reference_event_ratio": event_ratio,
        })
    return {
        "schema_version": 1,
        "pair_id": pair_id,
        "mode": mode,
        "reference_role": ("independent_reference_implementation" if mode == "t1"
                           else "original_program"),
        "reference_source": {key: reference[key] for key in
                             ("source_id", "source_sha256", "ir_sha256")},
        "candidate_source": {key: candidate[key] for key in
                             ("source_id", "source_sha256", "ir_sha256")},
        "inputs": rows,
        "equal_input_mean_rates": sums,
        "normalized_exposure": {
            "reference": ref_exposure,
            "candidate": cand_exposure,
            "candidate_minus_reference": cand_exposure - ref_exposure,
        },
    }


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", required=True, choices=("t1", "t3"),
                        help="T1 uses an independent reference; T3 uses the original program")
    parser.add_argument("--pair-id", required=True, help="your label for this matched pair")
    parser.add_argument("--reference", required=True, type=Path,
                        help="fi/summarize.py JSON for the reference")
    parser.add_argument("--candidate", required=True, type=Path,
                        help="fi/summarize.py JSON for the candidate")
    parser.add_argument("--reference-source-id")
    parser.add_argument("--candidate-source-id")
    args = parser.parse_args(argv)
    try:
        reference = read_summary(args.reference, args.reference_source_id)
        candidate = read_summary(args.candidate, args.candidate_source_id)
        result = compare(reference, candidate, args.mode, args.pair_id)
    except (OSError, ValueError, json.JSONDecodeError) as exc:
        parser.error(str(exc))
    json.dump(result, sys.stdout, indent=2)
    sys.stdout.write("\n")


if __name__ == "__main__":
    main()
