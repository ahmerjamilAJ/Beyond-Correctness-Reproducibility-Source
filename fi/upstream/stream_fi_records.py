"""Bounded-memory ledger readers;original injection/sampling code is unchanged."""
from collections import Counter
import hashlib
import json
from pathlib import Path


def read_records(path, dependencies_sha256):
    records = {}
    if not path.exists():
        return records
    with path.open() as stream:
        for line_number, line in enumerate(stream, 1):
            try:
                item = json.loads(line)
            except json.JSONDecodeError as error:
                raise RuntimeError(f'{path}:{line_number}: malformed JSONL;inspect before resume') from error
            if item['dependencies_sha256'] != dependencies_sha256:
                raise RuntimeError(f'{path}:{line_number}: dependency mismatch')
            if item['ordinal'] in records:
                raise RuntimeError(f'{path}:{line_number}: duplicate ordinal')
            if item.get('record_status') != 'complete':
                raise RuntimeError(f'{path}:{line_number}: noncomplete trial;inspect raw files before resume')
            # The original execution loop uses only ordinal membership.
            records[item['ordinal']] = None
    return records


def summarize_with(original_summary, path):
    # Preserve every field/formula in the original summarizer. Its input is a
    # tiny metadata-only projection in original order,not fault payloads.
    rows = []
    if path.exists():
        with path.open() as stream:
            for line in stream:
                if line.strip():
                    r = json.loads(line)
                    rows.append({k: r[k] for k in ['verdict', 'elapsed_s']} |
                                {'delivered_one_bit': r.get('delivered_one_bit', False)})
    class Projection:
        def read_text(self):
            return '\n'.join(json.dumps(r) for r in rows)
    return original_summary(Projection())


def install(base):
    original_summary = base.summarize
    original_trial = base._trial
    digest = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    base._read_records = read_records
    base.summarize = lambda path: summarize_with(original_summary, path)
    def trial(*args, **kwargs):
        record = original_trial(*args, **kwargs)
        record['bounded_memory_execution_sha256'] = digest
        return record
    base._trial = trial
    return {'change': 'Streaming payload reads and metadata-only ordinal/summary accumulation.',
            'adapter_sha256': digest, 'original_runner_sha256': hashlib.sha256(Path(base.__file__).read_bytes()).hexdigest(),
            'unchanged': 'Original _prepare,_trial injection logic,_run_cell persistence,sampler,opcode filters,bitflip model and outcome rules.'}
