#!/usr/bin/env python3
"""One-bit LLFI execution inside the documented Linux container.

The random condition samples an eligible *dynamic cycle* uniformly from LLFI's
profiled 1..total_cycle range. This is not a uniform sample of static IR sites.
LLFI chooses the bit within the selected result; its effective choice is read
from the raw stat, and can be replayed by specifying fi_bit separately.

Every attempted trial has a JSONL record.
Raw per-trial LLFI files are removed only after the record is appended, fsynced,
and validated; invalid/incomplete attempts retain their raw files for audit.
"""

from __future__ import annotations

import argparse
import base64
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import random
import re
import shutil
import signal
import subprocess
import sys
import time
import zlib
from typing import Any


SCHEMA_VERSION = 1
ROOT = Path(__file__).resolve().parents[2]
MAX_OUTPUT_BYTES = 16 * 1024 * 1024
OVERSIZE_SAMPLE_BYTES = 64 * 1024
STAT_RE = re.compile(r"^llfi\.stat\.fi\.injectedfaults\.0-0\.txt$")
CONTROL_ENV = {
    "LC_ALL": "C", "LANG": "C", "TZ": "UTC", "OMP_NUM_THREADS": "1",
    "OPENBLAS_NUM_THREADS": "1", "MKL_NUM_THREADS": "1",
}
# Both campaign modes operate on result-producing instructions. Control flow,
# memory stores, calls, allocation, and PHI nodes are outside the fault model.
# `dstreg` also makes the no-result exclusions noninjectable, but listing every
# opcode here keeps the LLFI configuration and paper policy identical.
EXCLUDED_OPCODES = ("ret", "alloca", "call", "phi", "store", "br", "switch", "unreachable")
def output_contract(workload: dict[str, Any], input_spec: dict[str, Any]) -> dict[str, Any]:
    """Validate the complete, deterministic output surface of one workload.

    LLFI moves newly created top-level files into baseline/prog_output and
    inserts `.prof` or `.0-0` before the last extension. Keep the manifest
    contract deliberately narrower than LLFI's general file handling.
    """
    output = workload.get("output")
    if not isinstance(output, dict) or output.get("comparator") != "exact_bytes":
        raise ValueError(f"{workload.get('id')}: a reviewed exact_bytes output contract is required")
    kind = output.get("kind")
    if kind == "stdout":
        if set(output) != {"kind", "comparator"}:
            raise ValueError("stdout output contract has unsupported fields")
        return {"kind": "stdout", "artifact_name": None, "stdout_oracle": None}
    if kind != "file" or set(output) != {"kind", "path", "comparator"}:
        raise ValueError("output kind must be stdout or a single top-level file")
    name = output["path"]
    if (not isinstance(name, str) or not name or name in {".", ".."}
            or Path(name).name != name or name.startswith("llfi") or "." not in name):
        raise ValueError("output file path must be a safe top-level basename with an extension")
    stdout_path = input_spec.get("stdout_oracle_path")
    if stdout_path is None:
        stdout_oracle = b""
    else:
        path = (ROOT / stdout_path).resolve()
        if ROOT not in path.parents or not path.is_file():
            raise ValueError("stdout oracle must be a repository file")
        stdout_oracle = path.read_bytes()
        if sha_bytes(stdout_oracle) != input_spec.get("stdout_oracle_sha256"):
            raise ValueError("stdout oracle SHA mismatch")
    if len(stdout_oracle) > MAX_OUTPUT_BYTES:
        raise ValueError("stdout oracle exceeds output size limit")
    return {"kind": "file", "artifact_name": name, "stdout_oracle": stdout_oracle}


def llfi_artifact_name(name: str, run_id: str) -> str:
    stem, extension = name.rsplit(".", 1)
    return f"{stem}.{run_id}.{extension}"


def sha_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def digest_json(value: Any) -> str:
    return sha_bytes(json.dumps(value, sort_keys=True, separators=(",", ":")).encode())


def atomic_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    with temporary.open("w") as stream:
        json.dump(value, stream, indent=2, sort_keys=True)
        stream.write("\n")
        stream.flush()
        os.fsync(stream.fileno())
    temporary.replace(path)


def encoded_bytes(value: bytes) -> dict[str, Any]:
    """Preserve exact bytes (including invalid UTF-8), compressed in the record."""
    return {
        "encoding": "zlib+base64", "size": len(value), "sha256": sha_bytes(value),
        "data": base64.b64encode(zlib.compress(value, level=6)).decode("ascii"),
    }


def oversized_file_evidence(path: Path) -> dict[str, Any]:
    """Fingerprint an oversized output without retaining its full contents."""
    size = path.stat().st_size
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        prefix = stream.read(OVERSIZE_SAMPLE_BYTES)
        digest.update(prefix)
        remaining = size - len(prefix)
        while remaining > 0:
            block = stream.read(min(1 << 20, remaining))
            if not block:
                raise RuntimeError(f"short read while hashing oversized output: {path}")
            digest.update(block)
            remaining -= len(block)
        if size > OVERSIZE_SAMPLE_BYTES:
            stream.seek(max(0, size - OVERSIZE_SAMPLE_BYTES))
            suffix = stream.read(OVERSIZE_SAMPLE_BYTES)
        else:
            suffix = prefix
    return {"size": size, "sha256": digest.hexdigest(),
            "prefix": encoded_bytes(prefix), "suffix": encoded_bytes(suffix)}


def check_encoded_bytes(payload: dict[str, Any]) -> None:
    decoded = zlib.decompress(base64.b64decode(payload["data"], validate=True))
    if len(decoded) != payload["size"] or sha_bytes(decoded) != payload["sha256"]:
        raise ValueError("compact output payload failed round-trip validation")


def parse_llfi_stat_bytes(raw: bytes, source: str) -> dict[str, Any]:
    """Parse exactly one LLFI delivery record, preserving unrecognized fields."""
    lines = raw.decode(errors="replace").splitlines()
    if len(lines) != 1 or not lines[0].startswith("FI stat: "):
        raise ValueError(f"{source}: expected exactly one FI stat line")
    fields: dict[str, Any] = {}
    for item in lines[0][len("FI stat: "):].split(", "):
        if "=" not in item:
            raise ValueError(f"{source}: malformed FI stat field {item!r}")
        key, value = item.split("=", 1)
        fields[key] = int(value) if re.fullmatch(r"-?\d+", value) else value
    for key in ("fi_type", "fi_index", "fi_cycle", "fi_reg_width", "fi_bit"):
        if key not in fields:
            raise ValueError(f"{source}: missing {key}")
    if fields["fi_type"] != "bitflip":
        raise ValueError(f"{source}: unexpected fi_type {fields['fi_type']!r}")
    width, bit = fields["fi_reg_width"], fields["fi_bit"]
    if not isinstance(width, int) or not isinstance(bit, int) or width <= 0 or not 0 <= bit < width:
        raise ValueError(f"{source}: invalid bit {bit!r} for width {width!r}")
    fields["effective_mask_hex"] = hex(1 << bit)
    fields["effective_bit_count"] = 1
    return fields


def parse_llfi_stat(path: Path) -> dict[str, Any]:
    return parse_llfi_stat_bytes(path.read_bytes(), str(path))


def enumerate_indexed_ir(path: Path) -> list[dict[str, Any]]:
    """Map LLVM metadata indices to static IR instructions; reachability is separate."""
    content = path.read_text(errors="replace").splitlines()
    metadata = {}
    for line in content:
        match = re.match(r"^!(\d+) = !\{i64 (\d+)\}$", line)
        if match:
            metadata[match.group(1)] = int(match.group(2))
    sites = []
    for line_number, line in enumerate(content, 1):
        match = re.search(r"!llfi_index !(\d+)(?:,|\s*$)", line)
        if not match or match.group(1) not in metadata:
            continue
        instruction = line.strip().split(", !llfi_index", 1)[0]
        rhs = instruction.split(" = ", 1)[-1]
        opcode = rhs.split(maxsplit=1)[0] if rhs else ""
        if opcode in ("tail", "musttail", "notail"):
            opcode = "call"
        sites.append({"index": metadata[match.group(1)], "opcode": opcode,
                      "ir_line": line_number})
    return sorted(sites, key=lambda site: site["index"])


def _run(command: list[str], cwd: Path, timeout_s: float) -> subprocess.CompletedProcess[bytes]:
    environment = os.environ.copy()
    environment.update(CONTROL_ENV)
    process = subprocess.Popen(command, cwd=cwd, env=environment, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, start_new_session=True)
    try:
        stdout, stderr = process.communicate(timeout=timeout_s)
    except subprocess.TimeoutExpired:
        # LLFI's Python wrappers spawn the actual benchmark. Kill the entire
        # process group, including that child, before any result is inspected.
        os.killpg(process.pid, signal.SIGKILL)
        process.communicate()
        raise
    return subprocess.CompletedProcess(command, process.returncode, stdout, stderr)


def _checked(command: list[str], cwd: Path, timeout_s: float, stage: str) -> subprocess.CompletedProcess[bytes]:
    try:
        result = _run(command, cwd, timeout_s)
    except subprocess.TimeoutExpired as error:
        raise RuntimeError(f"{stage} exceeded {timeout_s}s") from error
    if result.returncode != 0:
        detail = (result.stderr or result.stdout)[-4000:].decode(errors="replace")
        raise RuntimeError(f"{stage} failed (exit {result.returncode}): {detail}")
    return result


def _yaml(timeout_s: int, cycle: int | None, target_index: int | None) -> str:
    if target_index is None:
        excluded = ", ".join(EXCLUDED_OPCODES)
        selector = """  instSelMethod:
    - insttype:
        include: [all]
        exclude: [EXCLUDED]
""".replace("EXCLUDED", excluded)
    else:
        selector = f"""  instSelMethod:
    - customInstselector:
        include: [llfiindex]
        options: [-injecttoindex={target_index}]
"""
    cycle_line = "" if cycle is None else f"      fi_cycle: {cycle}\n"
    return (f"defaultTimeout: {timeout_s}\ncompileOption:\n{selector}"
            "  regSelMethod: regloc\n  regloc: dstreg\nrunOption:\n"
            "  - run:\n      numOfRuns: 1\n      fi_type: bitflip\n"
            "      fi_num_bits: 1\n" + cycle_line)


def _paths_for_args(args: list[str], work: Path) -> tuple[list[str], dict[str, str]]:
    translated = []
    inputs: dict[str, str] = {}
    for arg in args:
        candidate = Path(arg)
        source = (ROOT / candidate).resolve() if not candidate.is_absolute() else candidate.resolve()
        if source.is_file() and (source == ROOT or ROOT in source.parents):
            destination = work / source.name
            if destination.exists() and sha_file(destination) != sha_file(source):
                raise ValueError(f"input basename collision: {source.name}")
            shutil.copy2(source, destination)
            translated.append(source.name)
            inputs[arg] = sha_file(source)
        else:
            translated.append(arg)
    return translated, inputs


def verified_upstream_hashes(workload: dict[str, Any]) -> dict[str, str]:
    """Pin included upstream sources/headers declared by an adapter."""
    declared = workload.get("upstream_source_files_sha256", {})
    if not isinstance(declared, dict):
        raise ValueError("upstream_source_files_sha256 must be a path/hash mapping")
    verified = {}
    for name, expected in sorted(declared.items()):
        path = (ROOT / name).resolve()
        if ROOT not in path.parents or not path.is_file():
            raise ValueError(f"upstream source is absent or outside repository: {name}")
        actual = sha_file(path)
        if actual != expected:
            raise ValueError(f"upstream source hash mismatch: {name}")
        verified[name] = actual
    return verified


def _cell_dependencies(workload: dict[str, Any], input_spec: dict[str, Any],
                       target_index: int | None, timeout_s: int, seed: int,
                       runner_sha256: str | None = None) -> dict[str, Any]:
    source = (ROOT / workload["source"]).resolve()
    oracle = (ROOT / input_spec["oracle_path"]).resolve()
    if ROOT not in source.parents or ROOT not in oracle.parents:
        raise ValueError("source and oracle paths must stay inside repository")
    if sha_file(oracle) != input_spec["oracle_sha256"]:
        raise ValueError(f"oracle SHA mismatch: {oracle}")
    args = input_spec["args"]
    input_hashes = {}
    for arg in args:
        file_path = ROOT / arg
        if file_path.is_file():
            input_hashes[arg] = sha_file(file_path)
    return {
        "workload": workload, "input": input_spec, "source_sha256": sha_file(source),
        "verified_upstream_source_files_sha256": verified_upstream_hashes(workload),
        "oracle_sha256": sha_file(oracle), "input_file_sha256": input_hashes,
        "target_index": target_index, "timeout_s": timeout_s, "campaign_seed": seed,
        "runner_sha256": runner_sha256 or sha_file(Path(__file__)),
    }


def _prepare(workload: dict[str, Any], input_spec: dict[str, Any], cell: Path,
             deps_hash: str, clean_repeats: int, timeout_s: int,
             target_index: int | None,
             compatible_deps_hashes: set[str] | None = None) -> dict[str, Any]:
    work = cell / "work"
    metadata_path = cell / "metadata.json"
    if metadata_path.exists():
        metadata = json.loads(metadata_path.read_text())
        if (metadata.get("dependencies_sha256") != deps_hash and
                metadata.get("dependencies_sha256") not in (compatible_deps_hashes or set())):
            raise RuntimeError(f"{cell}: existing experiment has different dependencies")
        if metadata.get("clean_repeats", 0) < clean_repeats:
            raise RuntimeError(f"{cell}: existing clean gate has fewer than {clean_repeats} repeats")
        if not (work / "llfi" / "program-faultinjection.exe").is_file():
            raise RuntimeError(f"{cell}: prepared FI executable missing")
        return metadata
    if work.exists() and any(work.iterdir()):
        raise RuntimeError(f"{work}: unindexed prior work exists; inspect before reuse")
    work.mkdir(parents=True, exist_ok=True)
    source = (ROOT / workload["source"]).resolve()
    oracle_bytes = (ROOT / input_spec["oracle_path"]).read_bytes()
    contract = output_contract(workload, input_spec)
    expected_stdout = oracle_bytes if contract["kind"] == "stdout" else contract["stdout_oracle"]
    artifact_name = contract["artifact_name"]
    if len(oracle_bytes) > MAX_OUTPUT_BYTES:
        raise ValueError("oracle output exceeds pilot size limit")
    args, input_hashes = _paths_for_args(input_spec["args"], work)
    language = workload["language"].lower()
    compiler = "clang++-15" if language in ("c++", "cpp", "cxx") else "clang-15"
    flags = list(workload.get("compile_flags", []))
    linker_flags = list(workload.get("link_flags", ["-lm", "-lstdc++"] if compiler == "clang++-15" else ["-lm"]))
    (work / "input.yaml").write_text(_yaml(timeout_s, None, target_index))
    _checked([compiler, *flags, str(source), "-o", "program", *linker_flags], work,
             120, "native compile")
    _checked([compiler, *flags, "-S", "-emit-llvm", "-fno-discard-value-names",
              str(source), "-o", "program.ll"], work, 120, "LLVM IR compile")
    native_durations = []
    for repeat in range(clean_repeats):
        if artifact_name is not None:
            (work / artifact_name).unlink(missing_ok=True)
        start = time.monotonic()
        clean = _checked(["./program", *args], work, timeout_s, f"native clean {repeat}")
        native_durations.append(time.monotonic() - start)
        if clean.stdout != expected_stdout:
            raise RuntimeError(f"native clean {repeat}: stdout differs from its frozen oracle")
        if artifact_name is not None:
            native_artifact = work / artifact_name
            if not native_artifact.is_file() or native_artifact.read_bytes() != oracle_bytes:
                raise RuntimeError(f"native clean {repeat}: output file differs from independent oracle")
            native_artifact.unlink()
        if workload.get("stderr", "empty") == "empty" and clean.stderr:
            raise RuntimeError(f"native clean {repeat}: nonempty stderr")
    instrument = _checked(["instrument", "--readable", "program.ll", *linker_flags],
                          work, 180, "LLFI instrument")
    (work / "instrument.log").write_bytes(instrument.stdout + instrument.stderr)
    indexed_ir = work / "llfi" / "program-llfi_index.ll"
    if not indexed_ir.is_file():
        raise RuntimeError("LLFI did not emit indexed IR")
    sites = enumerate_indexed_ir(indexed_ir)
    if target_index is not None and target_index not in {site["index"] for site in sites}:
        raise RuntimeError(f"target LLFI index {target_index} is absent from indexed IR")
    atomic_json(cell / "sites.json", {"schema_version": SCHEMA_VERSION,
                                      "indexed_ir_sha256": sha_file(indexed_ir), "sites": sites})
    profile_cycles = []
    profile_durations = []
    for repeat in range(clean_repeats):
        if artifact_name is not None:
            (work / artifact_name).unlink(missing_ok=True)
            (work / "llfi" / "baseline" / llfi_artifact_name(artifact_name, "prof")).unlink(missing_ok=True)
        start = time.monotonic()
        prof = _checked(["profile", "./llfi/program-profiling.exe", *args], work,
                        timeout_s + 15, f"LLFI profile {repeat}")
        profile_durations.append(time.monotonic() - start)
        baseline = work / "llfi" / "baseline" / "golden_std_output"
        if not baseline.is_file() or baseline.read_bytes() != expected_stdout:
            raise RuntimeError(f"LLFI profile {repeat}: stdout differs from its frozen oracle")
        if artifact_name is not None:
            artifact = work / "llfi" / "baseline" / llfi_artifact_name(artifact_name, "prof")
            if not artifact.is_file() or artifact.read_bytes() != oracle_bytes:
                raise RuntimeError(f"LLFI profile {repeat}: output file differs from independent oracle")
            if (work / artifact_name).exists():
                raise RuntimeError(f"LLFI profile {repeat}: output file was not captured by LLFI")
        baseline_names = {path.name for path in baseline.parent.iterdir()}
        allowed_baseline = {"golden_std_output"}
        if artifact_name is not None:
            allowed_baseline.add(llfi_artifact_name(artifact_name, "prof"))
        if baseline_names != allowed_baseline:
            raise RuntimeError(f"LLFI profile {repeat}: uncontracted baseline files {baseline_names - allowed_baseline}")
        if workload.get("stderr", "empty") == "empty" and prof.stderr:
            raise RuntimeError(f"LLFI profile {repeat}: nonempty stderr")
        prof_stat = work / "llfi.stat.prof.txt"
        match = re.search(r"^total_cycle=(\d+)$", prof_stat.read_text(), re.MULTILINE)
        if not match:
            raise RuntimeError(f"LLFI profile {repeat}: total_cycle missing")
        profile_cycles.append(int(match.group(1)))
    if len(set(profile_cycles)) != 1:
        raise RuntimeError(f"unstable profile cycle counts: {profile_cycles}")
    if profile_cycles[0] <= 0:
        raise RuntimeError(f"zero profile cycle counts: {profile_cycles}")
    metadata = {
        "schema_version": SCHEMA_VERSION, "workload_id": workload["id"],
        "input_id": input_spec["id"], "mode": "random" if target_index is None else "per_instruction",
        "output_kind": contract["kind"], "artifact_name": artifact_name,
        "stdout_oracle_sha256": sha_bytes(expected_stdout),
        "target_index": target_index, "dependencies_sha256": deps_hash,
        "source_sha256": sha_file(source), "oracle_sha256": sha_bytes(oracle_bytes),
        "verified_upstream_source_files_sha256": verified_upstream_hashes(workload),
        "input_file_sha256": input_hashes, "manifest_input_args": input_spec["args"],
        "runtime_args": args, "compile_flags": flags, "link_flags": linker_flags,
        "binary_sha256": sha_file(work / "program"),
        "ir_sha256": sha_file(work / "program.ll"),
        "indexed_ir_sha256": sha_file(indexed_ir),
        "fi_binary_sha256": sha_file(work / "llfi" / "program-faultinjection.exe"),
        "selector_yaml_sha256": sha_file(work / "input.yaml"),
        "clean_repeats": clean_repeats, "native_duration_s": native_durations,
        "profile_duration_s": profile_durations, "total_cycle": profile_cycles[0],
        "timeout_s": timeout_s,
        "llvm_version": _checked([compiler, "--version"], work, 10, "compiler version").stdout.decode(errors="replace").splitlines()[0],
    }
    atomic_json(metadata_path, metadata)
    # Instrumentation intermediates are reproducible and unused after the FI
    # executable, indexed site map, and metadata have been persisted.  Keeping
    # two large IR copies per site/input exhausted the host during Suite64.
    for name in ("program-faultinjection.ll", "program-profiling.ll"):
        (work / "llfi" / name).unlink(missing_ok=True)
    return metadata


def _read_records(path: Path, dependencies_sha256: str) -> dict[int, dict[str, Any]]:
    records = {}
    if not path.exists():
        return records
    for line_number, line in enumerate(path.read_text().splitlines(), 1):
        try:
            item = json.loads(line)
        except json.JSONDecodeError as error:
            raise RuntimeError(f"{path}:{line_number}: malformed JSONL; inspect before resume") from error
        if item["dependencies_sha256"] != dependencies_sha256:
            raise RuntimeError(f"{path}:{line_number}: dependency mismatch")
        if item["ordinal"] in records:
            raise RuntimeError(f"{path}:{line_number}: duplicate ordinal")
        if item.get("record_status") != "complete":
            raise RuntimeError(f"{path}:{line_number}: noncomplete trial; inspect raw files before resume")
        records[item["ordinal"]] = item
    return records


def _clear_previous_outputs(work: Path) -> None:
    for directory in ("llfi_stat_output", "std_output", "error_output", "prog_output"):
        folder = work / "llfi" / directory
        folder.mkdir(exist_ok=True)
        leftovers = list(folder.iterdir())
        if leftovers:
            raise RuntimeError(f"unprocessed LLFI raw files remain in {folder}: {len(leftovers)}")
    runtime = work / "llfi.config.runtime.txt"
    if runtime.exists():
        raise RuntimeError(f"unprocessed LLFI runtime config remains: {runtime}")
    # The runtime writes and flushes this file before applying the bit flip.
    # injectfault normally moves it into llfi/llfi_stat_output.  If the fault
    # hangs or kills the wrapper, the exact record remains here instead.
    runtime_stat = work / "llfi.stat.fi.injectedfaults.txt"
    if runtime_stat.exists():
        raise RuntimeError(f"unprocessed LLFI runtime stat remains: {runtime_stat}")


def _cleanup_successful_raw(work: Path) -> None:
    for directory in ("llfi_stat_output", "std_output", "error_output", "prog_output"):
        folder = work / "llfi" / directory
        for path in folder.iterdir():
            if path.is_file():
                path.unlink()
    (work / "llfi.config.runtime.txt").unlink(missing_ok=True)
    (work / "llfi.stat.fi.injectedfaults.txt").unlink(missing_ok=True)


def _restore_input_files(metadata: dict[str, Any], work: Path) -> None:
    """Restore immutable input snapshots before/after every injected run."""
    for relative, expected in metadata["input_file_sha256"].items():
        source = (ROOT / relative).resolve()
        if ROOT not in source.parents or not source.is_file() or sha_file(source) != expected:
            raise RuntimeError(f"frozen input differs before trial: {relative}")
        destination = work / source.name
        shutil.copy2(source, destination)
        if sha_file(destination) != expected:
            raise RuntimeError(f"failed to restore trial input: {relative}")


def _input_mutations(metadata: dict[str, Any], work: Path) -> list[dict[str, Any]]:
    mutations = []
    for relative, expected in metadata["input_file_sha256"].items():
        path = work / Path(relative).name
        if not path.is_file():
            mutations.append({"path": relative, "expected_sha256": expected, "missing": True})
            continue
        actual = sha_file(path)
        if actual == expected:
            continue
        evidence = {"path": relative, "expected_sha256": expected,
                    "actual_sha256": actual, "size": path.stat().st_size}
        if path.stat().st_size > MAX_OUTPUT_BYTES:
            evidence["oversize"] = oversized_file_evidence(path)
        else:
            evidence["payload"] = encoded_bytes(path.read_bytes())
        mutations.append(evidence)
    return mutations


def _trial(metadata: dict[str, Any], work: Path, ordinal: int, seed: int,
           target_index: int | None) -> dict[str, Any]:
    _clear_previous_outputs(work)
    _restore_input_files(metadata, work)
    trial_seed = int(digest_json({"seed": seed, "cell": metadata["dependencies_sha256"],
                                  "ordinal": ordinal})[:16], 16)
    cycle = random.Random(trial_seed).randint(1, metadata["total_cycle"])
    yaml_content = _yaml(metadata["timeout_s"], cycle, target_index)
    (work / "input.yaml").write_text(yaml_content)
    requested = {"fi_type": "bitflip", "fi_num_bits": 1, "fi_cycle": cycle,
                 "target_index": target_index, "python_seed": trial_seed,
                 "cycle_sampling": "uniform_dynamic_cycle",
                 "llfi_bit_sampling": "internal_unseeded", "input_yaml_sha256": sha_bytes(yaml_content.encode())}
    started = time.monotonic()
    process: subprocess.CompletedProcess[bytes] | None = None
    driver_timed_out = False
    try:
        process = _run(["injectfault", "./llfi/program-faultinjection.exe",
                        *metadata["runtime_args"]], work, metadata["timeout_s"] + 20)
    except subprocess.TimeoutExpired:
        driver_timed_out = True
    elapsed = time.monotonic() - started
    llfi = work / "llfi"
    stat_dir = llfi / "llfi_stat_output"
    stat_files = [path for path in stat_dir.iterdir() if path.is_file()] if stat_dir.exists() else []
    runtime_stat_path = work / "llfi.stat.fi.injectedfaults.txt"
    output_entries = sorted((llfi / "prog_output").iterdir(), key=lambda path: path.name)
    if any(not path.is_file() for path in output_entries):
        raise RuntimeError("FI produced a non-file output artifact; raw files retained")
    output_files = [path.name for path in output_entries]
    artifact_name = metadata.get("artifact_name")
    expected_output_file = (llfi_artifact_name(artifact_name, "0-0")
                            if artifact_name is not None else None)
    extra_outputs = [name for name in output_files if name != expected_output_file]
    extra_artifacts = []
    extra_artifact_bytes = 0
    for name in extra_outputs:
        path = llfi / "prog_output" / name
        size = path.stat().st_size
        extra_artifact_bytes += min(size, MAX_OUTPUT_BYTES + 1)
        if size > MAX_OUTPUT_BYTES or extra_artifact_bytes > MAX_OUTPUT_BYTES:
            extra_artifacts.append({"name": name, "oversize": oversized_file_evidence(path)})
        else:
            extra_artifacts.append({"name": name, "payload": encoded_bytes(path.read_bytes())})
    artifact_path = llfi / "prog_output" / expected_output_file if expected_output_file else None
    artifact_oversize = (oversized_file_evidence(artifact_path)
                         if artifact_path and artifact_path.is_file() and
                         artifact_path.stat().st_size > MAX_OUTPUT_BYTES else None)
    artifact = (artifact_path.read_bytes() if artifact_path and artifact_path.is_file()
                and artifact_oversize is None else None)
    output_path = llfi / "std_output" / "std_outputfile-run-0-0"
    error_path = llfi / "error_output" / "errorfile-run-0-0"
    runtime_path = work / "llfi.config.runtime.txt"
    stdout_oversize = (oversized_file_evidence(output_path)
                       if output_path.is_file() and output_path.stat().st_size > MAX_OUTPUT_BYTES else None)
    output = output_path.read_bytes() if output_path.is_file() and stdout_oversize is None else None
    error = error_path.read_bytes() if error_path.is_file() else b""
    driver_stderr = process.stderr if process is not None else b""
    runtime_config = runtime_path.read_bytes() if runtime_path.is_file() else None
    input_mutations = _input_mutations(metadata, work)
    # The LLTFI runtime itself prints this line on a successful injection to
    # the child's inherited stderr. Preserve the original combined stream and
    # split out any benchmark diagnostic. A delivered fault that makes the
    # target report an error is a classified detected failure, even when a
    # buggy target returns zero or happens to emit the correct data artifact.
    diagnostic_lines = driver_stderr.splitlines()
    target_diagnostic_lines = [
        line for line in diagnostic_lines if line != b"MSG: injectFunc() has being called"
    ]
    target_stderr = b"\n".join(target_diagnostic_lines)
    if target_diagnostic_lines and driver_stderr.endswith(b"\n"):
        target_stderr += b"\n"
    llfi_runtime_diagnostic_only = bool(diagnostic_lines) and not target_diagnostic_lines
    # Auto-classify only diagnostics whose literal bytes are present in the
    # compiled target IR. Other mixed driver/target stderr still fails closed
    # for review rather than being guessed to be a benchmark detector.
    program_ir = (work / "program.ll").read_bytes()
    target_stderr_source_literal = bool(target_diagnostic_lines) and all(
        line in program_ir for line in target_diagnostic_lines
    )
    # miniFE assembles these diagnostics from two frozen string literals.  An
    # exact emitted line is target evidence even though the complete line is
    # absent from the IR as one contiguous literal.  Verify both fragments in
    # the compiled IR before accepting it as a detected failure.
    if not target_stderr_source_literal and metadata["workload_id"] == "minife":
        for first, second in (
            (b"miniFE::cg_solve ERROR, A.has_local_indices is false, needs to be true. This probably means ",
             b"miniFE::make_local_matrix(A) was not called prior to calling miniFE::cg_solve."),
            (b"miniFE::time_kernels ERROR, A.has_local_indices is false, needs to be true. This probably means ",
             b"miniFE::make_local_matrix(A) was not called prior to calling miniFE::time_kernels."),
        ):
            if target_stderr == first + second + b"\n" and first in program_ir and second in program_ir:
                target_stderr_source_literal = True
                break
    if len(error) > MAX_OUTPUT_BYTES or len(driver_stderr) > MAX_OUTPUT_BYTES:
        raise RuntimeError("FI stderr exceeds pilot size limit; raw files retained")
    stat = None
    stat_raw = None
    delivery_evidence = None
    parse_error = None
    if len(stat_files) == 1 and STAT_RE.match(stat_files[0].name) and not runtime_stat_path.exists():
        try:
            stat_raw = stat_files[0].read_bytes()
            stat = parse_llfi_stat_bytes(stat_raw, str(stat_files[0]))
            delivery_evidence = "injectfault_moved_stat"
        except ValueError as exc:
            parse_error = str(exc)
    elif not stat_files and runtime_stat_path.is_file():
        # LLFI flushes this exact record immediately before injectFaultImpl.
        # A killed/hung wrapper can therefore leave valid delivery evidence in
        # the working directory without moving it to llfi_stat_output.
        try:
            stat_raw = runtime_stat_path.read_bytes()
            stat = parse_llfi_stat_bytes(stat_raw, str(runtime_stat_path))
            delivery_evidence = "llfi_runtime_stat_before_wrapper_move"
        except ValueError as exc:
            parse_error = str(exc)
    else:
        parse_error = ("expected exactly one moved or pre-move LLFI stat; "
                       f"moved={[path.name for path in stat_files]}, "
                       f"pre_move={runtime_stat_path.exists()}")
    delivered = bool(stat and stat["fi_cycle"] == cycle and
                     (target_index is None or stat["fi_index"] == target_index))
    if runtime_config is None or f"fi_cycle={cycle}\n".encode() not in runtime_config:
        delivered = False
        parse_error = (parse_error + "; " if parse_error else "") + "missing/mismatched LLFI runtime cycle config"
    error_lower = error.lower()
    return_match = re.search(rb"return code (-?\d+)", error)
    post_delivery_timeout = bool(delivered and (
        driver_timed_out or process is not None and process.returncode == -9
        and elapsed >= metadata["timeout_s"]
    ))
    target_exit_code = (None if post_delivery_timeout else
                        int(return_match.group(1)) if return_match else
                        (0 if not error and output is not None else None))
    if post_delivery_timeout:
        verdict = "timeout"
    elif driver_timed_out:
        verdict = "invalid_driver_timeout"
    elif process is None or process.returncode != 0:
        verdict = "invalid_driver_error"
    elif not delivered:
        verdict = "invalid_not_injected"
    elif b"hang" in error_lower or b"timed out" in error_lower:
        verdict = "timeout"
    elif error:
        verdict = "crash"
    elif target_stderr and target_stderr_source_literal:
        verdict = "detected_failure"
    elif target_stderr:
        verdict = "invalid_unclassified_stderr"
    elif output is None and stdout_oversize is None:
        verdict = "invalid_missing_stdout"
    elif input_mutations or extra_outputs or stdout_oversize is not None or artifact_oversize is not None:
        # A normal exit that writes any undeclared file violates the complete
        # frozen output contract. Preserve those bytes and classify the
        # delivered fault as an observable silent data corruption.
        verdict = "sdc"
    elif (sha_bytes(output) == metadata.get("stdout_oracle_sha256", metadata["oracle_sha256"])
          and (metadata.get("output_kind", "stdout") == "stdout"
               or artifact is not None and sha_bytes(artifact) == metadata["oracle_sha256"])):
        verdict = "correct_masked"
    else:
        verdict = "sdc"
    valid = verdict in {"timeout", "crash", "detected_failure", "correct_masked", "sdc"}
    record = {
        "schema_version": SCHEMA_VERSION, "record_status": "complete" if valid else "needs_review",
        "ordinal": ordinal, "trial_id": f"{metadata['workload_id']}:{metadata['input_id']}:{target_index}:{ordinal}",
        "dependencies_sha256": metadata["dependencies_sha256"],
        "source_sha256": metadata["source_sha256"], "binary_sha256": metadata["fi_binary_sha256"],
        "ir_sha256": metadata["ir_sha256"], "input_file_sha256": metadata["input_file_sha256"],
        "oracle_sha256": metadata["oracle_sha256"], "requested": requested,
        "runtime_config": encoded_bytes(runtime_config) if runtime_config is not None else None,
        "delivered": stat, "delivered_one_bit": delivered,
        "stat_raw": encoded_bytes(stat_raw) if stat_raw is not None else None,
        "delivery_evidence": delivery_evidence,
        "parse_error": parse_error, "extra_outputs": extra_outputs,
        "uncontracted_artifacts": extra_artifacts, "verdict": verdict,
        "input_mutations": input_mutations,
        "output_kind": metadata.get("output_kind", "stdout"),
        "artifact_name": artifact_name,
        "artifact": encoded_bytes(artifact) if artifact is not None else None,
        "artifact_oversize": artifact_oversize,
        "driver_exit_code": process.returncode if process is not None else None,
        "target_exit_code": target_exit_code,
        "target_signal": -target_exit_code if target_exit_code is not None and target_exit_code < 0 else None,
        "target_timed_out": post_delivery_timeout or b"hang" in error_lower or b"timed out" in error_lower,
        "driver_timed_out": driver_timed_out, "elapsed_s": elapsed,
        "stdout": encoded_bytes(output) if output is not None else None,
        "stdout_oversize": stdout_oversize,
        "llfi_error": encoded_bytes(error), "driver_stderr": encoded_bytes(driver_stderr),
        "target_stderr": encoded_bytes(target_stderr),
        "target_stderr_source_literal": target_stderr_source_literal,
        "llfi_runtime_diagnostic_only": llfi_runtime_diagnostic_only,
        "detector_state": ("target_reported_error" if verdict == "detected_failure" else
                           "input_side_effect" if input_mutations and verdict == "sdc" else
                           "exact_output_contract_violation" if extra_outputs and verdict == "sdc" else
                           "unclassified"),
        "recovery_state": "not_claimed",
    }
    for key in ("stdout", "artifact", "llfi_error", "driver_stderr", "target_stderr",
                "runtime_config", "stat_raw"):
        if record[key] is not None:
            check_encoded_bytes(record[key])
    for descriptor in (record["stdout_oversize"], record["artifact_oversize"]):
        if descriptor is not None:
            check_encoded_bytes(descriptor["prefix"])
            check_encoded_bytes(descriptor["suffix"])
    for item in record["uncontracted_artifacts"]:
        if "payload" in item:
            check_encoded_bytes(item["payload"])
        else:
            check_encoded_bytes(item["oversize"]["prefix"])
            check_encoded_bytes(item["oversize"]["suffix"])
    for item in record["input_mutations"]:
        if "payload" in item:
            check_encoded_bytes(item["payload"])
        elif "oversize" in item:
            check_encoded_bytes(item["oversize"]["prefix"])
            check_encoded_bytes(item["oversize"]["suffix"])
    return record


def summarize(records_path: Path) -> dict[str, Any]:
    """Report attempted/delivered accounting and outcome counts only."""
    records = [json.loads(line) for line in records_path.read_text().splitlines() if line]
    verdicts = Counter(record["verdict"] for record in records)
    delivered = sum(bool(record.get("delivered_one_bit")) for record in records)
    count = len(records)
    return {
        "schema_version": SCHEMA_VERSION, "attempted": count, "delivered": delivered,
        "not_delivered_or_invalid": count - delivered, "verdict_counts": dict(sorted(verdicts.items())),
    }


def _run_cell(workload: dict[str, Any], input_spec: dict[str, Any], result_dir: Path,
              trials: int, clean_repeats: int, timeout_s: int, seed: int,
              target_index: int | None, prepare_only: bool = False) -> dict[str, Any]:
    mode = "random" if target_index is None else f"per_instruction/index_{target_index}"
    cell = result_dir / mode / workload["id"] / input_spec["id"]
    cell.mkdir(parents=True, exist_ok=True)
    lock = cell / ".runner.lock"
    try:
        lock_fd = os.open(lock, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
    except FileExistsError as error:
        raise RuntimeError(f"{cell}: another runner appears active; inspect {lock}") from error
    try:
        os.write(lock_fd, f"pid={os.getpid()}\n".encode())
        os.close(lock_fd)
        dependencies = _cell_dependencies(workload, input_spec, target_index, timeout_s, seed)
        deps_hash = digest_json(dependencies)
        metadata = _prepare(workload, input_spec, cell, deps_hash, clean_repeats,
                            timeout_s, target_index)
        deps_hash = metadata["dependencies_sha256"]
        if prepare_only:
            return {"workload_id": workload["id"], "input_id": input_spec["id"],
                    "target_index": target_index, "prepared": True,
                    "total_cycle": metadata["total_cycle"],
                    "dependencies_sha256": deps_hash}
        records_path = cell / "trials.jsonl"
        existing = _read_records(records_path, deps_hash)
        for ordinal in range(trials):
            if ordinal in existing:
                continue
            record = _trial(metadata, cell / "work", ordinal, seed, target_index)
            record["runner_sha256"] = sha_file(Path(__file__))
            serialised = json.dumps(record, sort_keys=True, separators=(",", ":")) + "\n"
            with records_path.open("a") as stream:
                stream.write(serialised)
                stream.flush()
                os.fsync(stream.fileno())
            if record["record_status"] != "complete":
                raise RuntimeError(f"{cell}: trial {ordinal} needs review ({record['verdict']}); raw files retained")
            # Re-read the exact persisted record before deleting raw output.
            with records_path.open("rb") as stream:
                stream.seek(-len(serialised.encode()), os.SEEK_END)
                confirmed = json.loads(stream.read())
            if confirmed["trial_id"] != record["trial_id"] or confirmed["dependencies_sha256"] != deps_hash:
                raise RuntimeError(f"{cell}: persisted trial does not match in-memory trial")
            for key in ("stdout", "artifact", "llfi_error", "driver_stderr", "target_stderr",
                        "runtime_config", "stat_raw"):
                if confirmed[key] is not None:
                    check_encoded_bytes(confirmed[key])
            _cleanup_successful_raw(cell / "work")
            _restore_input_files(metadata, cell / "work")
        summary = summarize(records_path)
        summary.update({"workload_id": workload["id"], "input_id": input_spec["id"],
                        "target_index": target_index, "dependencies_sha256": deps_hash})
        if target_index is not None:
            summary["rate_basis"] = "uniform sampled dynamic occurrences of selected LLFI instruction index"
        atomic_json(cell / "summary.json", summary)
        return summary
    finally:
        lock.unlink(missing_ok=True)


def run_campaign(manifest_path: Path, result_dir: Path, *, trials: int, seed: int,
                 workload_ids: list[str] | None = None,
                 input_ids: list[str] | None = None,
                 clean_repeats: int = 20, timeout_s: int = 10,
                 target_index: int | None = None,
                 prepare_only: bool = False) -> list[dict[str, Any]]:
    """Run bounded, sequential cells. Call from inside the project LLFI container."""
    if sys.platform != "linux" or not Path("/opt/LLTFI/_install/bin/injectfault").is_file():
        raise RuntimeError("FI must run inside the LLFI project container")
    if trials < 1 or clean_repeats < 2 or timeout_s < 1 or target_index is not None and target_index < 0:
        raise ValueError("trials>=1, clean_repeats>=2, timeout_s>=1, target_index>=0 required")
    manifest = json.loads(Path(manifest_path).read_text())
    if manifest.get("schema_version") != 1:
        raise ValueError("unsupported benchmark manifest schema")
    selected = [item for item in manifest["workloads"]
                if workload_ids is None or item["id"] in workload_ids]
    if workload_ids is not None and {item["id"] for item in selected} != set(workload_ids):
        raise ValueError("unknown workload id requested")
    summaries = []
    matched_input_ids: set[str] = set()
    for workload in selected:
        for input_spec in workload["inputs"]:
            if input_ids is not None and input_spec["id"] not in input_ids:
                continue
            matched_input_ids.add(input_spec["id"])
            output_contract(workload, input_spec)
            summaries.append(_run_cell(workload, input_spec, Path(result_dir), trials,
                                       clean_repeats, timeout_s, seed, target_index,
                                       prepare_only=prepare_only))
    if input_ids is not None and matched_input_ids != set(input_ids):
        raise ValueError(f"unknown input id(s) for selected workloads: {sorted(set(input_ids) - matched_input_ids)}")
    return summaries


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--results", type=Path, required=True)
    parser.add_argument("--workload", action="append", dest="workloads")
    parser.add_argument("--input", action="append", dest="inputs")
    parser.add_argument("--trials", type=int, required=True)
    parser.add_argument("--clean-repeats", type=int, default=20)
    parser.add_argument("--timeout", type=int, default=10)
    parser.add_argument("--seed", type=int, required=True)
    parser.add_argument("--target-index", type=int)
    parser.add_argument("--prepare-only", action="store_true",
                        help="Run clean/native/profile gates and persist indexed IR without FI trials")
    args = parser.parse_args()
    summaries = run_campaign(args.manifest, args.results, workload_ids=args.workloads,
                             input_ids=args.inputs,
                             trials=args.trials, clean_repeats=args.clean_repeats,
                             timeout_s=args.timeout, seed=args.seed,
                             target_index=args.target_index, prepare_only=args.prepare_only)
    print(json.dumps(summaries, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (RuntimeError, ValueError) as error:
        print(f"random_fi: {error}", file=sys.stderr)
        raise SystemExit(2)
