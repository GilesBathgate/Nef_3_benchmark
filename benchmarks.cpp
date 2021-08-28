#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/IO/Nef_polyhedron_iostream_3.h>

#include <benchmark/benchmark.h>

#include <fstream>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef CGAL::Surface_mesh<K::Point_3> Surface_mesh;
typedef CGAL::Nef_polyhedron_3<K> Nef_polyhedron;

static void BM_difference(benchmark::State& state) {
  for (auto _ : state) {
    Surface_mesh A, B;
    {
      std::ifstream in("data/spheregrid.off");
      in >> A;
    }

    {
      std::ifstream in("data/shiftedspheregrid.off");
      in >> B;
    }

    Nef_polyhedron nefA(A), nefB(B);

    Nef_polyhedron nefC = nefA.difference(nefB);
  }
}

static void BM_intersection(benchmark::State& state) {
  for (auto _ : state) {
    Surface_mesh A, B;
    {
      std::ifstream in("data/spheregrid.off");
      in >> A;
    }

    {
      std::ifstream in("data/shiftedspheregrid.off");
      in >> B;
    }

    Nef_polyhedron nefA(A), nefB(B);

    Nef_polyhedron nefC = nefA.intersection(nefB);
  }
}

BENCHMARK(BM_difference)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_intersection)->Unit(benchmark::kMillisecond);
BENCHMARK_MAIN();
