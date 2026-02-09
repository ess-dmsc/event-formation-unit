#!/bin/bash
# Helper script to run all benchmarks with standardized options
# Usage: benchmark_run.sh <benchmarks_dir> <results_dir> [repetitions]

set -e

BENCHMARKS_DIR="${1:-build/benchmarks}"
RESULTS_DIR="${2:-build/benchmark_results}"
REPETITIONS="${3:-3}"

if [ -z "$BENCHMARKS_DIR" ] || [ -z "$RESULTS_DIR" ]; then
    echo "Usage: $0 <benchmarks_dir> <results_dir> [repetitions]"
    exit 1
fi

mkdir -p "$RESULTS_DIR"

echo "Running all benchmarks..."
echo ""

for benchmark in "$BENCHMARKS_DIR"/*; do
  if [[ -x "$benchmark" && -f "$benchmark" ]]; then
    BENCHMARK_NAME=$(basename "$benchmark")
    echo "Running benchmark: ${BENCHMARK_NAME}"
    "$benchmark" \
      --benchmark_out="${RESULTS_DIR}/${BENCHMARK_NAME}.json" \
      --benchmark_out_format=json \
      --benchmark_repetitions="${REPETITIONS}" \
      | tee "${RESULTS_DIR}/${BENCHMARK_NAME}.txt"
    echo ""
  fi
done

echo "Aggregating benchmark results..."
python3 "$(dirname "$0")/benchmark_aggregate.py" "$RESULTS_DIR"
