#!/bin/bash

# Ensure build is up to date
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target constructor_benchmark

# Run constructor benchmarks and output to results file
echo "Running Constructor Benchmarks..." > benchmark_results.txt
for i in {0..5}
do
    echo "Comparing CreateEdgeFacetOverlay Case $i..." >> benchmark_results.txt
    /usr/bin/python3 ../benchmark/tools/compare.py filters build/constructor_benchmark "BM_CreateEdgeFacetOverlay_Baseline/$i" "BM_CreateEdgeFacetOverlay_Contender/$i" --benchmark_repetitions=9 2> /dev/null | tee -a benchmark_results.txt
done

