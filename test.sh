#!/bin/bash

BENCHMARKS=("Intersect_EE" "Intersect_EF" "Intersect_SE" "Intersect_SF" "Intersect_RE" "Intersect_RF")

for b in "${BENCHMARKS[@]}"
do
    echo "Comparing $b..."
    /usr/bin/python3 ../benchmark/tools/compare.py filters build/intersection_benchmark "BM_${b}_Baseline" "BM_${b}_Contender" --benchmark_repetitions=9 2> /dev/null | grep OVERALL_GEOMEAN
done
