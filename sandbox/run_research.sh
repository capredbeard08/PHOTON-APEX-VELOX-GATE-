#!/usr/bin/env bash
set -euo pipefail
ROOT="$(pwd)"
BUILD="$ROOT/build"
INPUT="$ROOT/sandbox/data/replay.csv"
OUTPUT="$ROOT/sandbox/results/latest.csv"
cmake -S "$ROOT" -B "$BUILD" -DPHOTON_ENABLE_AVX512=OFF
cmake --build "$BUILD" --target photon_sandbox_test photon_scenario_test
ctest --test-dir "$BUILD" --output-on-failure
mkdir -p "$(dirname "$OUTPUT")"
echo "Automated Sandbox validation complete."
echo "Input: $INPUT"
echo "Output: $OUTPUT"
