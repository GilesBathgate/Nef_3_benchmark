#!/bin/bash

# Exit on error
set -e

# 1. Rebuild the benchmarks
echo "Rebuilding benchmarks..."
cmake -B build
cmake --build build

# 2. Run the benchmarks with stability measures
echo "Running benchmarks..."
taskset -c 0 setarch $(uname -m) -R ./build/k3_tree_benchmark \
    --benchmark_filter="BM_K3_tree_build_contender|BM_K3_tree_build_improved" \
    --benchmark_repetitions=9 \
    --benchmark_enable_random_interleaving=true \
    --benchmark_format=json > results.json

# 3. Generate comparison report
echo "Generating comparison report..."
# Using /usr/bin/python3 to ensure we use the version with numpy/scipy installed
python3 ../benchmark/tools/compare.py filters results.json BM_K3_tree_build_contender BM_K3_tree_build_improved > benchmark_results.txt

# 4. Remove ANSI color codes
echo "Removing ANSI color codes..."
sed 's/\x1b\[[0-9;]*m//g' -i ./benchmark_results.txt

# 5. Clean up
rm results.json

echo "Benchmark completed. Results saved in benchmark_results.txt"
tail -n 1 benchmark_results.txt
