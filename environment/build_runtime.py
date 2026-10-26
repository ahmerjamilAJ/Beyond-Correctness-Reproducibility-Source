#!/usr/bin/env python3
"""Build the included serial generic LLFI runtime in a new output directory."""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]

def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--out',type=Path,required=True)
    p.add_argument('--jobs',type=int,default=1)
    a=p.parse_args()
    if sys.platform!='linux': p.error('Build and use the FI runtime on Linux.')
    if a.jobs<1:p.error('--jobs must be positive')
    src=ROOT/'fi/runtime';out=a.out.resolve()
    if out.exists() and any(out.iterdir()):p.error('Choose a new, empty output directory.')
    out.mkdir(parents=True,exist_ok=True)
    subprocess.run(['cmake','-S',str(src),'-B',str(out),'-DCMAKE_BUILD_TYPE=Release'],check=True)
    subprocess.run(['cmake','--build',str(out),'--target','llfi-rt','-j',str(a.jobs)],check=True)
    library=out/'libllfi-rt.so'
    symbols=subprocess.run(['readelf','-Ws',str(library)],check=True,capture_output=True,text=True).stdout
    undefined=[s for s in symbols.splitlines() if ' UND ' in s]
    if any(' rand@' in s or ' srand@' in s for s in undefined):
        raise RuntimeError('Shared application RNG dependency remains.')
    if not any(' rand_r@' in s for s in undefined):
        raise RuntimeError('Private-state RNG symbol is absent.')
    identity={'variant':'private_rand_r_v1',
              'scope':'serial generic LLFI, fresh process per injection',
              'library_sha256':sha(library),
              'source_sha256':{q.name:sha(q) for q in sorted(src.iterdir()) if q.is_file()},
              'unsupported':'ML-specific static runtime and concurrent injection threads'}
    (out/'build_identity.json').write_text(json.dumps(identity,indent=2)+'\n')
    print(library)

if __name__=='__main__':main()
