#!/usr/bin/env python3
"""Set paths for the pinned LLTFI checkout; patch installed PyYAML entry points."""
from pathlib import Path
import argparse

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source',type=Path,required=True)
    p.add_argument('--llvm',type=Path)
    p.add_argument('--installed',type=Path)
    a=p.parse_args()
    if a.llvm:
        llvm=a.llvm.resolve()
        if not (llvm/'include/llvm/IR/Instruction.def').is_file():
            p.error('--llvm must be an installed LLVM development prefix')
        (a.source/'config/llvm_paths.cmake').write_text(
            f'set(LLVM_DST_ROOT "{llvm}")\nset(LLVM_SRC_ROOT "{llvm}")\n')
        (a.source/'config/llvm_paths.py').write_text(
            f'LLVM_DST_ROOT = {str(llvm)!r}\nLLVM_SRC_ROOT = {str(llvm)!r}\nLLVM_GXX_BIN_DIR = {str(llvm/"bin")!r}\n')
    if a.installed:
        for name in ['instrument','profile','injectfault','batchInstrument','batchProfile','batchInjectfault']:
            q=a.installed/'bin'/name;text=q.read_text()
            if text.count('yaml.load(')!=1:
                raise RuntimeError(f'{name}: pinned PyYAML call no longer matches')
            q.write_text(text.replace('yaml.load(','yaml.safe_load(',1))

if __name__=='__main__':main()
