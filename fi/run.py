#!/usr/bin/env python3
"""Fresh random or targeted FI with an explicit runtime contract."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import re
import runpy
import subprocess
import sys
import tarfile

ROOT=Path(__file__).resolve().parents[1]
UPSTREAM=ROOT/'fi/upstream'

def sha(path):
    h=hashlib.sha256()
    with Path(path).open('rb') as f:
        for block in iter(lambda:f.read(1<<20),b''):h.update(block)
    return h.hexdigest()

def immutable(path,value):
    payload=json.dumps(value,indent=2,sort_keys=True)+'\n'
    if path.exists():
        if path.read_text()!=payload:raise RuntimeError('Existing execution contract differs; use a new output directory.')
    else:
        with path.open('x') as f:f.write(payload)

def load_program_configuration(program):
    if not re.fullmatch(r'[A-Za-z0-9_-]+',program):
        raise ValueError('Use a benchmark folder name for --program.')
    path=ROOT/'benchmarks/programs'/program/'run.yaml'
    if not path.is_file():
        raise ValueError('No runnable configuration for '+program+'. Check benchmarks/catalog.tsv.')
    try:
        import yaml
    except ImportError as error:
        raise ValueError('Run FI in the supplied Linux environment, which includes PyYAML.') from error
    config=yaml.safe_load(path.read_text())
    if not isinstance(config,dict) or not isinstance(config.get('workloads'),list) or not config['workloads']:
        raise ValueError('Invalid benchmark configuration: '+str(path))
    return config,path

def install_runtime_guard(base,library,retain_raw):
    original_prepare=base._prepare
    original_trial=base._trial
    def prepare(*args,**kwargs):
        metadata=original_prepare(*args,**kwargs)
        cell=Path(args[2]);exe=cell/'work/llfi/program-faultinjection.exe'
        linked=subprocess.run(['ldd',str(exe)],check=True,capture_output=True,text=True).stdout
        found=re.search(r'^\s*libllfi-rt\.so\s+=>\s+(\S+)',linked,re.M)
        if not found or Path(found[1]).resolve()!=library:
            raise RuntimeError('Prepared executable is not using the requested runtime.')
        return metadata
    def trial(metadata,work,ordinal,seed,target_index):
        record=original_trial(metadata,work,ordinal,seed,target_index)
        record['execution_runtime']={'variant':'private_rand_r_v1','library_sha256':sha(library)}
        record['requested']['llfi_bit_sampling']='process_private_rand_r_bounded_rejection'
        raw=record.get('stat_raw')
        marker_ok=False
        if raw:
            import base64,zlib
            data=zlib.decompress(base64.b64decode(raw['data']))
            marker_ok=bool(re.search(rb'bc_rng_variant=private_rand_r_v1, bc_rng_initial_seed=\d+, bc_rng_draw_count=\d+',data))
        record['execution_runtime']['delivery_marker_verified']=marker_ok
        if record.get('delivered_one_bit') and not marker_ok:
            record['record_status']='needs_review'
            record['verdict']='invalid_runtime_identity'
        if retain_raw:
            cell=Path(work).parent;folder=cell/'raw';folder.mkdir(exist_ok=True)
            archive=folder/f'trial_{ordinal:09d}.tar.gz'
            # Per-trial output/config/stat bytes are kept independently of the
            # compact ledger. Compiled binaries and IR are retained once per cell.
            candidates=['input.yaml','llfi.config.runtime.txt','llfi.stat.fi.injectedfaults.txt',
                        'llfi.stat.fi.injectedfaults.0-0.txt',
                        'llfi/llfi_stat_output','llfi/prog_output','llfi/error_output','llfi/std_output',
                        'llfi/llfi.config.runtime.txt']
            with tarfile.open(archive,'x:gz',dereference=False) as tar:
                for name in candidates:
                    p=Path(work)/name
                    if p.exists():tar.add(p,arcname=name,recursive=True)
            with archive.open('rb') as f:os.fsync(f.fileno())
            record['raw_archive']={'path':str(archive.relative_to(cell)), 'sha256':sha(archive)}
        return record
    base._prepare=prepare
    base._trial=trial

def main():
    p=argparse.ArgumentParser(description=__doc__)
    source=p.add_mutually_exclusive_group(required=True)
    source.add_argument('--program',help='Benchmark folder name, such as polybench_bicg.')
    source.add_argument('--manifest',type=Path,help='Advanced: an existing machine-format workload configuration.')
    p.add_argument('--results',type=Path,required=True)
    p.add_argument('--trials',type=int,required=True,help='Your fixed trials per source/input or targeted site/input.')
    p.add_argument('--seed',type=int,required=True,help='Seed for dynamic-cycle sampling; runtime entropy seeds are recorded separately.')
    p.add_argument('--clean-repeats',type=int,default=20)
    p.add_argument('--timeout',type=int,default=15)
    p.add_argument('--workload',action='append')
    p.add_argument('--input',action='append')
    scope=p.add_mutually_exclusive_group()
    scope.add_argument('--target-index',type=int)
    scope.add_argument('--all-sites',action='store_true')
    p.add_argument('--prepare-only',action='store_true')
    p.add_argument('--runtime-library',type=Path,default=Path(os.environ.get('BC_LLFI_RUNTIME','/opt/bc-runtime/libllfi-rt.so')))
    p.add_argument('--compact-only',action='store_true',help='Keep exact compact ledger evidence without additional per-trial raw archives.')
    a=p.parse_args()
    if a.trials<1 or a.seed<0 or a.clean_repeats<2 or a.timeout<1:p.error('Positive budget/timeout, nonnegative seed and at least two clean repeats are required.')
    if a.all_sites and a.input:p.error('--all-sites uses the inputs in the supplied manifest; create a subset manifest instead of using --input.')
    if a.all_sites and a.prepare_only:p.error('Prepare random mode first, then enumerate all targeted sites.')
    config=config_path=None
    if a.program:
        try:config,config_path=load_program_configuration(a.program)
        except ValueError as error:p.error(str(error))
    library=a.runtime_library.resolve()
    identity_path=library.parent/'build_identity.json'
    if not library.is_file() or not identity_path.is_file():p.error('Build the included runtime first; library and build_identity.json are required.')
    identity=json.loads(identity_path.read_text())
    if identity['variant']!='private_rand_r_v1' or identity['library_sha256']!=sha(library):
        p.error('Runtime identity does not match the build receipt.')
    source_hashes=identity.get('source_sha256',{})
    source_names={path.name for path in (ROOT/'fi/runtime').iterdir() if path.is_file()}
    if not isinstance(source_hashes,dict) or set(source_hashes)!=source_names:
        p.error('Runtime build identity must cover every included runtime source file.')
    if any(not (ROOT/'fi/runtime'/name).is_file() or sha(ROOT/'fi/runtime'/name)!=digest
           for name,digest in source_hashes.items()):
        p.error('Included runtime sources differ from the selected runtime build.')
    if os.environ.get('LD_PRELOAD'):p.error('Use an environment without LD_PRELOAD.')
    # Only this process and its children inherit the selected runtime.
    old=os.environ.get('LD_LIBRARY_PATH','')
    os.environ['LD_LIBRARY_PATH']=str(library.parent)+(':'+old if old else '')
    out=a.results.resolve()
    if out==ROOT or ROOT not in out.parents:p.error('Write new experiment outputs inside this folder, normally runs/.')
    if out.exists() and any(out.iterdir()) and not (out/'execution_contract.json').is_file():
        p.error('Existing output has no matching execution contract; use a new directory.')
    if a.manifest:
        manifest=a.manifest.resolve()
        if ROOT not in manifest.parents:p.error('Keep the configuration and source/input/oracle files inside the reproducibility folder.')
    out.mkdir(parents=True,exist_ok=True)
    if a.program:
        manifest=out/'workload.json'
        immutable(manifest,config)
    contract={'manifest_sha256':sha(manifest),'runtime':identity,
              'runner_sha256':sha(Path(__file__)),'mode':'all_sites' if a.all_sites else 'targeted' if a.target_index is not None else 'random',
              'trials':a.trials,'seed':a.seed,'clean_repeats':a.clean_repeats,'timeout':a.timeout,
              'workloads':a.workload,'inputs':a.input,
              'target_index':a.target_index,'retain_raw':not a.compact_only}
    if config_path:contract['program_configuration_sha256']=sha(config_path)
    immutable(out/'execution_contract.json',contract)
    sys.path.insert(0,str(UPSTREAM))
    import random_fi as base
    from stream_fi_records import install
    install(base)
    install_runtime_guard(base,library,not a.compact_only)
    if a.all_sites:
        name='per_instruction.py'
        argv=[str(UPSTREAM/name),'--manifest',str(manifest),'--results',str(out),
              '--trials',str(a.trials),'--seed',str(a.seed),'--clean-repeats',str(a.clean_repeats),'--timeout',str(a.timeout)]
        for value in a.workload or []:argv+=['--workload',value]
        argv+=['--all-sites','--site-source','clean_profile']
        sys.argv=argv
        runpy.run_path(str(UPSTREAM/name),run_name='__main__')
    else:
        summaries=base.run_campaign(manifest,out,workload_ids=a.workload,input_ids=a.input,
                     trials=a.trials,clean_repeats=a.clean_repeats,timeout_s=a.timeout,seed=a.seed,
                     target_index=a.target_index,prepare_only=a.prepare_only)
        print(json.dumps(summaries,indent=2))

if __name__=='__main__':main()
