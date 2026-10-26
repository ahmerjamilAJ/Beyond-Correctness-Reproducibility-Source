# Beyond Correctness: companion methods

This repository contains final prompts, reported model settings, a CPU benchmark catalog with available source and inputs, generic LLVM fault-injection scripts, and basic analysis. It contains no model responses, generated programs, original fault-injection results, or inference code.

## Benchmarks

The paper uses 56 CPU test programs (54 benchmark families) and six development programs. `benchmarks/catalog.tsv` lists all 62 program names and marks the available code and run configurations. Some benchmark code or inputs may not be shared due to licensing issues. Each supplied `run.yaml` records build settings, four inputs, and clean-output checks.

## Models

| Primary model | Quantization |
| --- | --- |
| Devstral Small 2 (24B) | Q4_K_M |
| Gemma 4 A4B (26B total) | Q4_K_M |
| Qwen3-30B-A3B-Instruct-2507 | Q4_K_M |
| gpt-oss 120B | MXFP4 |

The separate commercial-chat study used GPT-5.4 Thinking, Gemini 3 Thinking, and Claude Sonnet 4.6 Adaptive. Primary models were served on an M3 Ultra Mac Studio with Docker Model Runner v1.2.1 and llama.cpp Metal revision `ac4cdde`.

## Files and use

- `prompts/task1`, `task2`, and `task3` contain the final templates. Replace their `{{PLACEHOLDER}}` fields before use.
- `environment/` defines AArch64 Linux with LLVM 15 and LLTFI revision `66dad0d3314ab72eeb82384fc21066f88615b358`.
- `fi/run.py` runs available CPU configurations; `fi/summarize.py` exports outcomes from new runs.
- `analysis/metrics.py` computes basic rates and normalized exposure from new FI summaries.

From the repository root:

```sh
docker build --platform linux/arm64 -f environment/Dockerfile -t beyond-correctness-fi .
docker run --platform linux/arm64 --rm -it -v "$PWD:/workspace" beyond-correctness-fi bash
python3 -B fi/run.py --program polybench_bicg --results runs/bicg --trials 5000 --seed 1
python3 -B fi/summarize.py --results runs/bicg --output runs/bicg-summary.json
python3 -B analysis/metrics.py --help
```

The example seed starts a new run; it is not a reported study seed. Applicable third-party notices are in source headers, `benchmarks/catalog.tsv`, `benchmarks/licenses/`, and `licenses/`.
