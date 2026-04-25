#include <CGAL/Simple_cartesian.h>
#include <CGAL/Nef_3/SNC_k3_tree_traits.h>
#include <CGAL/Nef_3/K3_tree.h>
#include "K3_tree_contender.h"
#include "K3_tree_improved.h"
#include "Mock_SNC_structure.h"
#include <CGAL/Random.h>
#include <CGAL/point_generators_3.h>
#include <benchmark/benchmark.h>
#include <vector>

Mock_SNC_structure global_snc;

static void BM_K3_tree_build_baseline(benchmark::State& state) {
  for (auto _ : state) {
    CGAL::K3_tree<CGAL::SNC_k3_tree_traits<Mock_decorator>> tree(&global_snc);
    benchmark::DoNotOptimize(tree);
  }
}

static void BM_K3_tree_build_contender(benchmark::State& state) {
  for (auto _ : state) {
    Contender::K3_tree<CGAL::SNC_k3_tree_traits<Mock_decorator>> tree(&global_snc);
    benchmark::DoNotOptimize(tree);
  }
}

static void BM_K3_tree_build_improved(benchmark::State& state) {
  for (auto _ : state) {
    Improved::K3_tree<CGAL::SNC_k3_tree_traits<Mock_decorator>> tree(&global_snc);
    benchmark::DoNotOptimize(tree);
  }
}

BENCHMARK(BM_K3_tree_build_baseline)->MinTime(2.0);
BENCHMARK(BM_K3_tree_build_contender)->MinTime(2.0);
BENCHMARK(BM_K3_tree_build_improved)->MinTime(2.0);

int main(int argc, char** argv) {
  populate(global_snc, 512);
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
