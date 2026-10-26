#!/usr/bin/env python3
"""Run fixed-LLFI-index fault injection with a locked site/input selection.

The preferred clean_profile mode screens static sites in deterministic opcode
round-robin order and verifies input-specific reachability with no-fault LLFI
profiles. The legacy random_prefix mode uses a fixed prefix of delivered
random-FI trials. Neither is an unbiased sample of every static instruction.
Each selected index conditions LLFI on a dynamic occurrence at that site, so
its outcome rate must remain separate from the random dynamic-event FI rate.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import hashlib
import json
import os
from pathlib import Path
import shutil
import sys
import time
from typing import Any

from random_fi import EXCLUDED_OPCODES, ROOT, atomic_json, run_campaign, sha_file


EXCLUDED_OPCODE = set(EXCLUDED_OPCODES)
SCHEMA_VERSION = 1


def honor_boundary_pause(results: Path) -> None:
    """Pause between static sites when the bounded parallel filler asks.

    Pausing at this boundary avoids freezing an active trial's wall-clock
    timeout.  The request names its controller PID so a stale file cannot
    strand a later campaign.
    """
    request = results / "per_instruction" / ".parallel_pause_request.json"
    acknowledgement = results / "per_instruction" / ".parallel_paused.json"
    if not request.is_file():
        acknowledgement.unlink(missing_ok=True)
        return
    try:
        controller_pid = int(json.loads(request.read_text())["controller_pid"])
    except (KeyError, TypeError, ValueError, json.JSONDecodeError):
        raise RuntimeError(f"{request}: invalid parallel pause request")
    if not Path(f"/proc/{controller_pid}").exists():
        request.unlink(missing_ok=True)
        acknowledgement.unlink(missing_ok=True)
        return
    atomic_json(acknowledgement, {
        "schema_version": 1, "controller_pid": controller_pid,
        "serial_worker_pid": os.getpid(), "state": "paused_at_site_boundary",
    })
    while request.exists() and Path(f"/proc/{controller_pid}").exists():
        time.sleep(0.25)
    acknowledgement.unlink(missing_ok=True)


def remove_unselected_screening_cells(result_dir: Path, selection: dict[str, Any]) -> dict[str, int]:
    """Delete clean-profile scratch cells that are outside the frozen FI frame.

    Clean-profile screening prepares a cell before it knows whether the site is
    reachable.  Those preparation directories contain compiled/profile data,
    but no FI trials.  Keeping them makes the targeted audit interpret them as
    unplanned FI cells and needlessly consumes disk space.
    """
    expected = {
        (site["workload_id"], input_id, int(site["index"]))
        for site in selection["selection"]
        if site.get("kind") != "evidence"
        for input_id in site["input_ids"]
    }
    removed_cells = 0
    removed_bytes = 0
    targeted_root = result_dir / "per_instruction"
    for cell in sorted(targeted_root.glob("index_*/*/*")):
        if not cell.is_dir():
            continue
        index = int(cell.parts[-3].removeprefix("index_"))
        pair = (cell.parts[-2], cell.parts[-1], index)
        if pair in expected:
            continue
        trials = cell / "trials.jsonl"
        summary = cell / "summary.json"
        if summary.exists() or (trials.exists() and trials.stat().st_size):
            raise RuntimeError(
                f"{cell}: unselected cell contains FI results; refusing automatic cleanup")
        removed_bytes += sum(path.stat().st_size for path in cell.rglob("*") if path.is_file())
        shutil.rmtree(cell)
        removed_cells += 1
    for directory in sorted(targeted_root.glob("index_*/*"), reverse=True):
        if directory.is_dir() and not any(directory.iterdir()):
            directory.rmdir()
    for directory in sorted(targeted_root.glob("index_*"), reverse=True):
        if directory.is_dir() and not any(directory.iterdir()):
            directory.rmdir()
    return {"removed_cells": removed_cells, "removed_bytes": removed_bytes}


def _selection_score(seed: int, workload_id: str, index: int) -> str:
    return hashlib.sha256(f"{seed}:{workload_id}:{index}".encode()).hexdigest()


def _opcode_round_robin(sites: dict[int, dict[str, Any]], seed: int,
                        workload_id: str) -> list[int]:
    """Balance the no-fault static-site screen across LLVM opcode strata."""
    buckets: dict[str, list[int]] = defaultdict(list)
    for index, site in sites.items():
        opcode = site["opcode"]
        if opcode not in EXCLUDED_OPCODE:
            buckets[opcode].append(index)
    for indices in buckets.values():
        indices.sort(key=lambda index: _selection_score(seed, workload_id, index))
    opcodes = sorted(buckets, key=lambda opcode: hashlib.sha256(
        f"{seed}:{workload_id}:opcode:{opcode}".encode()).hexdigest())
    ordered: list[int] = []
    while any(buckets.values()):
        for opcode in opcodes:
            if buckets[opcode]:
                ordered.append(buckets[opcode].pop(0))
    return ordered


def _random_evidence(result_dir: Path, workload: dict[str, Any],
                     evidence_trials: int) -> tuple[dict[int, set[str]], dict[int, dict[str, Any]], list[dict[str, Any]]]:
    observed: dict[int, set[str]] = defaultdict(set)
    sites: dict[int, dict[str, Any]] | None = None
    evidence = []
    for input_spec in workload["inputs"]:
        input_id = input_spec["id"]
        cell = result_dir / "random" / workload["id"] / input_id
        summary_path = cell / "summary.json"
        records_path = cell / "trials.jsonl"
        sites_path = cell / "sites.json"
        metadata_path = cell / "metadata.json"
        if not all(path.is_file() for path in (summary_path, records_path, sites_path, metadata_path)):
            raise RuntimeError(f"{cell}: complete random-FI pilot required before site selection")
        summary = json.loads(summary_path.read_text())
        metadata = json.loads(metadata_path.read_text())
        site_data = json.loads(sites_path.read_text())
        if summary["attempted"] < evidence_trials:
            raise RuntimeError(f"{cell}: need {evidence_trials} attempts, found {summary['attempted']}")
        current_sites = {int(site["index"]): site for site in site_data["sites"]}
        if sites is None:
            sites = current_sites
        elif sites != current_sites:
            raise RuntimeError(f"{workload['id']}: indexed IR sites differ across inputs")
        selected_records = []
        with records_path.open() as stream:
            for line in stream:
                record = json.loads(line)
                if record["ordinal"] >= evidence_trials:
                    continue
                if record.get("record_status") != "complete":
                    raise RuntimeError(f"{cell}: incomplete random attempt in evidence prefix")
                if record.get("delivered_one_bit"):
                    index = int(record["delivered"]["fi_index"])
                    if index not in current_sites:
                        raise RuntimeError(f"{cell}: delivered index {index} absent from indexed IR")
                    observed[index].add(input_id)
                    selected_records.append({"ordinal": record["ordinal"], "fi_index": index,
                                             "fi_cycle": record["delivered"]["fi_cycle"]})
        if len(selected_records) < evidence_trials:
            raise RuntimeError(f"{cell}: {len(selected_records)}/{evidence_trials} delivered attempts")
        evidence.append({"input_id": input_id,
                         "random_dependencies_sha256": metadata["dependencies_sha256"],
                         "indexed_ir_sha256": site_data["indexed_ir_sha256"],
                         "delivery_prefix_sha256": hashlib.sha256(
                             json.dumps(selected_records, sort_keys=True, separators=(",", ":")).encode()
                         ).hexdigest(),
                         "evidence_trials": evidence_trials})
    assert sites is not None
    return observed, sites, evidence


def select_sites(manifest_path: Path, result_dir: Path, *, workload_ids: list[str] | None,
                 max_sites: int, evidence_trials: int, seed: int,
                 include_unselected: bool = False) -> dict[str, Any]:
    if max_sites < 1 or evidence_trials < 1:
        raise ValueError("max_sites and evidence_trials must be positive")
    manifest = json.loads(manifest_path.read_text())
    if manifest.get("schema_version") != 1:
        raise ValueError("unsupported benchmark manifest")
    workloads = [workload for workload in manifest["workloads"]
                 if (include_unselected or workload.get("selected_for_targeted_fi")) and
                 (workload_ids is None or workload["id"] in workload_ids)]
    if workload_ids is not None and {workload["id"] for workload in workloads} != set(workload_ids):
        raise ValueError("requested workload not selected in manifest")
    selected = []
    for workload in workloads:
        observed, sites, evidence = _random_evidence(result_dir, workload, evidence_trials)
        candidates = [index for index, inputs in observed.items()
                      if sites[index]["opcode"] not in EXCLUDED_OPCODE and inputs]
        candidates.sort(key=lambda index: _selection_score(seed, workload["id"], index))
        for index in candidates[:max_sites]:
            selected.append({"workload_id": workload["id"], "index": index,
                             "opcode": sites[index]["opcode"],
                             "input_ids": sorted(observed[index]),
                             "selection_score": _selection_score(seed, workload["id"], index)})
        if not candidates:
            raise RuntimeError(f"{workload['id']}: no observed injectable site; inspect random trial records")
        selected.append({"workload_id": workload["id"], "kind": "evidence",
                         "random_input_evidence": evidence,
                         "observed_candidate_count": len(candidates),
                         "selected_site_count": min(max_sites, len(candidates))})
    return {"schema_version": SCHEMA_VERSION, "method": "hash_order_of_observed_random_delivery_sites",
            "population_limit": "sites observed in the fixed random-FI evidence prefix; no outcome-based selection",
            "seed": seed, "max_sites_per_workload": max_sites,
            "include_unselected": include_unselected,
            "evidence_trials_per_input": evidence_trials,
            "manifest_sha256": sha_file(manifest_path), "selection": selected}


def select_sites_clean_profile(manifest_path: Path, result_dir: Path, *,
                               workload_ids: list[str] | None, max_sites: int | None,
                               screen_limit: int | None, clean_repeats: int,
                               timeout_s: int, seed: int,
                               include_unselected: bool = False) -> dict[str, Any]:
    """Select static sites by a predeclared hash order and clean reachability.

    A targeted LLFI *profile* establishes reachability separately for every
    input. Sites reached by any input are retained, with an explicit mask of
    reachable cells. No FI outcome or random-FI trial is read. Prepared target
    cells are reused when FI begins.
    """
    if max_sites is not None and max_sites < 1:
        raise ValueError("max_sites must be positive or None for the complete site frame")
    if screen_limit is not None and screen_limit < 1:
        raise ValueError("screen_limit must be positive or None")
    if max_sites is not None and screen_limit is not None and screen_limit < max_sites:
        raise ValueError("screen_limit must be at least max_sites")
    manifest = json.loads(manifest_path.read_text())
    if manifest.get("schema_version") != 1:
        raise ValueError("unsupported benchmark manifest")
    workloads = [item for item in manifest["workloads"]
                 if (include_unselected or item.get("selected_for_targeted_fi")) and
                 (workload_ids is None or item["id"] in workload_ids)]
    if workload_ids is not None and {item["id"] for item in workloads} != set(workload_ids):
        raise ValueError("requested workload not selected in manifest")
    selected: list[dict[str, Any]] = []
    screening: list[dict[str, Any]] = []
    manifest_digest = sha_file(manifest_path)
    for workload in workloads:
        workload_id = workload["id"]
        input_ids = [item["id"] for item in workload["inputs"]]
        input_specs = {item["id"]: item for item in workload["inputs"]}
        source_digest = sha_file(ROOT / workload["source"])
        if not input_ids or len(input_ids) != len(set(input_ids)):
            raise ValueError(f"{workload_id}: duplicate or absent input ids")
        run_campaign(manifest_path, result_dir, workload_ids=[workload_id],
                     trials=1, clean_repeats=clean_repeats, timeout_s=timeout_s,
                     seed=seed, prepare_only=True)
        broad_sites: dict[int, dict[str, Any]] | None = None
        broad_evidence = []
        for input_id in input_ids:
            broad_cell = result_dir / "random" / workload_id / input_id
            site_data = json.loads((broad_cell / "sites.json").read_text())
            current = {int(site["index"]): site for site in site_data["sites"]}
            if broad_sites is None:
                broad_sites = current
            elif current != broad_sites:
                raise RuntimeError(f"{workload_id}: indexed IR sites differ across inputs")
            metadata = json.loads((broad_cell / "metadata.json").read_text())
            broad_evidence.append({"input_id": input_id,
                                   "dependencies_sha256": metadata["dependencies_sha256"],
                                   "indexed_ir_sha256": site_data["indexed_ir_sha256"],
                                   "eligible_dynamic_events": metadata["total_cycle"]})
        assert broad_sites is not None
        opcode_frame = Counter(site["opcode"] for site in broad_sites.values()
                               if site["opcode"] not in EXCLUDED_OPCODE)
        candidates = _opcode_round_robin(broad_sites, seed, workload_id)
        accepted = 0
        screened_candidates = candidates if screen_limit is None else candidates[:screen_limit]
        for screen_ordinal, index in enumerate(screened_candidates):
            if screen_ordinal % 100 == 0:
                atomic_json(result_dir / 'per_instruction/clean_profile_progress.json', {
                    'schema_version': 1, 'stage': 'no_fault_reachability_screening',
                    'workload_id': workload_id, 'screened_candidates': screen_ordinal,
                    'candidate_total': len(screened_candidates),
                    'reachable_sites_seen': accepted, 'next_index': index,
                    'progress_basis': 'actual completed clean site/input profiles; no FI outcomes used',
                })
            profile_evidence = []
            unreachable_inputs = []
            for input_id in input_ids:
                input_spec = input_specs[input_id]
                identity = {
                    "schema_version": 1, "manifest_sha256": manifest_digest,
                    "source_sha256": source_digest, "workload_id": workload_id,
                    "input_id": input_id, "target_index": index,
                    "oracle_sha256": sha_file(ROOT / input_spec["oracle_path"]),
                    "input_file_sha256": {
                        arg: sha_file(ROOT / arg) for arg in input_spec["args"]
                        if (ROOT / arg).is_file()
                    },
                    "seed": seed, "timeout_s": timeout_s,
                    "clean_repeats": clean_repeats,
                }
                cell = result_dir / "per_instruction" / f"index_{index}" / workload_id / input_id
                checkpoint = cell / "unreachable.json"
                if checkpoint.is_file():
                    prior = json.loads(checkpoint.read_text())
                    if (prior.get("identity") != identity or
                            prior.get("profile_cycles") != [0] * clean_repeats):
                        raise RuntimeError(f"{checkpoint}: no-fault reachability checkpoint differs")
                    unreachable_inputs.append(input_id)
                    continue
                try:
                    prepared = run_campaign(manifest_path, result_dir,
                                            workload_ids=[workload_id], input_ids=[input_id],
                                            trials=1, clean_repeats=clean_repeats,
                                            timeout_s=timeout_s, seed=seed,
                                            target_index=index, prepare_only=True)[0]
                except RuntimeError as error:
                    # Only two stable zero counts establish no-fault
                    # unreachability.  Persist that result before removing
                    # large, reproducible instrumenter scratch, so a Docker
                    # restart does not reprofile every unreachable cell.
                    if not str(error).startswith("zero profile cycle counts: "):
                        raise
                    cycles = json.loads(str(error).split(": ", 1)[1])
                    if cycles != [0] * clean_repeats or (cell / ".runner.lock").exists():
                        raise RuntimeError(f"{cell}: zero-profile evidence is incomplete") from error
                    atomic_json(checkpoint, {"schema_version": 1, "identity": identity,
                                             "profile_cycles": cycles})
                    shutil.rmtree(cell / "work")
                    (cell / "sites.json").unlink(missing_ok=True)
                    unreachable_inputs.append(input_id)
                    continue
                profile_evidence.append({"input_id": input_id,
                                         "target_dynamic_occurrences": prepared["total_cycle"],
                                         "dependencies_sha256": prepared["dependencies_sha256"]})
            screening.append({"workload_id": workload_id, "index": index,
                              "opcode": broad_sites[index]["opcode"],
                              "screen_ordinal": screen_ordinal,
                              "selection_score": _selection_score(seed, workload_id, index),
                              "reachable_inputs": [entry["input_id"] for entry in profile_evidence],
                              "unreachable_inputs": unreachable_inputs})
            if profile_evidence:
                selected.append({"workload_id": workload_id, "index": index,
                                 "opcode": broad_sites[index]["opcode"],
                                 "input_ids": [entry["input_id"] for entry in profile_evidence],
                                 "unreachable_input_ids": unreachable_inputs,
                                 "screen_ordinal": screen_ordinal,
                                 "selection_score": _selection_score(seed, workload_id, index),
                                 "clean_profile_evidence": profile_evidence})
                accepted += 1
            if max_sites is not None and accepted >= max_sites:
                break
        if accepted == 0:
            raise RuntimeError(f"{workload_id}: no reachable site in first {screen_limit} candidates")
        selected.append({"workload_id": workload_id, "kind": "evidence",
                         "broad_clean_profile_evidence": broad_evidence,
                         "screened_candidate_count": sum(row["workload_id"] == workload_id for row in screening),
                         "selected_site_count": accepted,
                         "eligible_static_candidate_count": len(candidates),
                         "eligible_opcode_counts": dict(sorted(opcode_frame.items())),
                         "selected_opcode_counts": dict(sorted(Counter(
                             row["opcode"] for row in selected
                             if row.get("workload_id") == workload_id and "index" in row).items()))})
    return {"schema_version": SCHEMA_VERSION,
            "method": "opcode_round_robin_clean_profiled_site_input_cells",
            "population_limit": ("all opcode-balanced eligible static LLFI indices"
                                 if max_sites is None else
                                 "first screen_limit opcode-balanced eligible static LLFI indices") +
                                "; eligible inputs require nonzero no-fault occurrence; no FI outcomes used",
            "seed": seed, "max_sites_per_workload": max_sites,
            "selected_workload_ids": sorted(item["id"] for item in workloads),
            "screen_limit_per_workload": screen_limit,
            "include_unselected": include_unselected,
            "clean_repeats": clean_repeats, "timeout_s": timeout_s,
            "manifest_sha256": sha_file(manifest_path),
            "screening": screening, "selection": selected}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--results", type=Path, required=True)
    parser.add_argument("--workload", action="append", dest="workloads")
    parser.add_argument("--include-unselected", action="store_true",
                        help="Allow an explicitly supplied candidate manifest outside the development pilot")
    parser.add_argument("--max-sites", type=int, default=8)
    parser.add_argument("--all-sites", action="store_true",
                        help="Clean-profile and inject every eligible reachable static site")
    parser.add_argument("--site-source", choices=("random_prefix", "clean_profile"),
                        required=True)
    parser.add_argument("--screen-limit", type=int, default=None,
                        help="Maximum hash-ordered static indices to profile per workload")
    parser.add_argument("--evidence-trials", type=int,
                        help="Explicit evidence-prefix budget when --site-source=random_prefix")
    parser.add_argument("--trials", type=int, required=True)
    parser.add_argument("--clean-repeats", type=int, default=20)
    parser.add_argument("--timeout", type=int, default=10)
    parser.add_argument("--seed", type=int, required=True)
    args = parser.parse_args()
    if args.site_source == "random_prefix" and not args.evidence_trials:
        parser.error("--evidence-trials is required for --site-source=random_prefix")
    if args.all_sites and args.site_source != "clean_profile":
        parser.error("--all-sites requires --site-source clean_profile")
    if args.all_sites and args.screen_limit is not None:
        parser.error("--screen-limit cannot be combined with --all-sites")
    effective_max_sites = None if args.all_sites else args.max_sites
    effective_screen_limit = None if args.all_sites else (args.screen_limit or 3 * args.max_sites)
    selection_path = args.results / "per_instruction" / "selection.json"
    if args.site_source == "clean_profile" and selection_path.exists():
        proposed = json.loads(selection_path.read_text())
        manifest = json.loads(args.manifest.read_text())
        requested_ids = sorted(item["id"] for item in manifest["workloads"]
                               if (args.include_unselected or item.get("selected_for_targeted_fi"))
                               and (args.workloads is None or item["id"] in args.workloads))
        if (proposed.get("method") != "opcode_round_robin_clean_profiled_site_input_cells"
                or proposed.get("manifest_sha256") != sha_file(args.manifest)
                or proposed.get("max_sites_per_workload") != effective_max_sites
                or proposed.get("screen_limit_per_workload") != effective_screen_limit
                or proposed.get("seed") != args.seed
                or proposed.get("clean_repeats") != args.clean_repeats
                or proposed.get("timeout_s") != args.timeout
                or proposed.get("include_unselected") != args.include_unselected):
            raise RuntimeError(f"{selection_path}: selection lock differs; use a new results directory")
        if proposed.get("selected_workload_ids") != requested_ids:
            raise RuntimeError(f"{selection_path}: workload selection differs; use a new results directory")
    else:
        if args.site_source == "clean_profile":
            proposed = select_sites_clean_profile(
                args.manifest, args.results, workload_ids=args.workloads,
                max_sites=effective_max_sites, screen_limit=effective_screen_limit,
                clean_repeats=args.clean_repeats, timeout_s=args.timeout,
                seed=args.seed, include_unselected=args.include_unselected)
        else:
            proposed = select_sites(args.manifest, args.results, workload_ids=args.workloads,
                                    max_sites=args.max_sites, evidence_trials=args.evidence_trials,
                                    seed=args.seed, include_unselected=args.include_unselected)
        if selection_path.exists():
            locked = json.loads(selection_path.read_text())
            if locked != proposed:
                raise RuntimeError(f"{selection_path}: selection lock differs; use a new results directory")
        else:
            atomic_json(selection_path, proposed)
    if args.site_source == "clean_profile":
        cleanup = remove_unselected_screening_cells(args.results, proposed)
        atomic_json(args.results / "per_instruction" / "screening_cleanup.json",
                    {"schema_version": SCHEMA_VERSION, **cleanup})
    summaries = []
    errors = []
    for site in proposed["selection"]:
        if site.get("kind") == "evidence":
            continue
        honor_boundary_pause(args.results)
        try:
            result = run_campaign(args.manifest, args.results,
                                  workload_ids=[site["workload_id"]], input_ids=site["input_ids"],
                                  trials=args.trials, clean_repeats=args.clean_repeats,
                                  timeout_s=args.timeout, seed=args.seed, target_index=site["index"])
            summaries.extend(result)
        except (RuntimeError, ValueError) as error:
            errors.append({"workload_id": site["workload_id"], "index": site["index"],
                           "input_ids": site["input_ids"], "error": str(error)})
            atomic_json(args.results / "per_instruction" / "errors.json", errors)
    honor_boundary_pause(args.results)
    atomic_json(args.results / "per_instruction" / "summary.json",
                {"schema_version": SCHEMA_VERSION, "selection_path": str(selection_path),
                 "completed_cells": len(summaries), "failed_sites": len(errors),
                 "summaries": summaries, "errors": errors})
    print(json.dumps({"completed_cells": len(summaries), "failed_sites": len(errors)}, sort_keys=True))
    return 2 if errors else 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (RuntimeError, ValueError) as error:
        print(f"per_instruction: {error}", file=sys.stderr)
        raise SystemExit(2)
