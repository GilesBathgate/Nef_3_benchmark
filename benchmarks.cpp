#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/IO/Nef_polyhedron_iostream_3.h>
#include <CGAL/Aff_transformation_3.h>
#include <benchmark/benchmark.h>

#include <fstream>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef CGAL::Surface_mesh<K::Point_3> Surface_mesh;
typedef CGAL::Nef_polyhedron_3<K> Nef_polyhedron;
typedef CGAL::Aff_transformation_3<K> Aff_transformation_3;
typedef CGAL::Vector_3<K> Vector_3;

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

static void BM_union(benchmark::State& state) {
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

    Nef_polyhedron nefC = nefA.join(nefB);
  }
}

static void BM_union_translate(benchmark::State& state) {
  for (auto _ : state) {
    Surface_mesh A, B;
    {
      std::ifstream in("data/sphere.off");
      in >> A;
    }

    {
      std::ifstream in("data/cube.off");
      in >> B;
    }

    Nef_polyhedron nefA(A), nefB(B);

    Nef_polyhedron nefC = nefA.join(nefB);

    Vector_3 v(10.0,10.0,10.0);
    Aff_transformation_3 t(
      1.0,0.0,0.0,v.x(),
      0.0,1.0,0.0,v.y(),
      0.0,0.0,1.0,v.z(),1.0);

    nefA.transform(t);
    nefB.transform(t);
    nefC.transform(t);
  }
}

BENCHMARK(BM_difference)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_intersection)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_union)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_union_translate)->Unit(benchmark::kMillisecond);
BENCHMARK_MAIN();
