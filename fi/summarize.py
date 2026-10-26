#!/usr/bin/env python3
"""Export fresh per-cell outcome counts from compact FI ledgers."""
from pathlib import Path
from collections import Counter
import argparse
import json

VALID={'correct_masked','sdc','detected_failure','crash','timeout'}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--results',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();cells=[]
    for path in sorted(a.results.rglob('trials.jsonl')):
        meta_path=path.parent/'metadata.json'
        if not meta_path.is_file():raise ValueError(f'Missing metadata for FI ledger {path}')
        m=json.loads(meta_path.read_text());counts=Counter();ordinals=set();attempted=delivered=0
        scope_path=path.parent/'scope_instrumentation.json'
        scope=json.loads(scope_path.read_text()) if scope_path.is_file() else None
        with path.open() as f:
            for line in f:
                if not line.strip():continue
                t=json.loads(line)
                if t['ordinal'] in ordinals:raise ValueError(f'Duplicate trial ordinal in {path}')
                ordinals.add(t['ordinal']);attempted+=1
                if t['dependencies_sha256']!=m['dependencies_sha256']:raise ValueError(f'Mixed dependency identities in {path}')
                if t.get('delivered_one_bit'):
                    delivered+=1
                    counts[t['verdict'] if t.get('record_status')=='complete' and t['verdict'] in VALID else 'unclassified']+=1
        cells.append({'source_id':m['workload_id'],'input_id':m['input_id'],'target_index':m['target_index'],
                      'sampling_mode':'per_instruction' if m['target_index'] is not None else 'random_dynamic_event',
                      'fault_scope':'function' if scope is not None else 'whole_program',
                      'scope_functions':scope.get('functions',[]) if scope is not None else [],
                      'n':delivered,'attempted':attempted,'counts':dict(counts),
                      'all_deliveries_classified':counts['unclassified']==0,
                      'dynamic_events':m['total_cycle'],'source_sha256':m['source_sha256'],
                      'ir_sha256':m['ir_sha256'],'ledger':str(path.resolve())})
    a.output.parent.mkdir(parents=True,exist_ok=True)
    with a.output.open('x') as f:json.dump({'schema_version':1,'cells':cells},f,indent=2);f.write('\n')
    print(a.output)

if __name__=='__main__':main()
