#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/enum.h>
#include <CGAL/Random.h>
#include <vector>
#include <algorithm>
#include <benchmark/benchmark.h>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point_3;

struct Vertex {
    Point_3 p;
    Vertex(Point_3 p) : p(p) {}
    const Point_3& point() const { return p; }
};

typedef Vertex* Vertex_handle;
typedef std::vector<Vertex_handle> Vertex_list;
typedef Vertex_list::iterator Vertex_iterator;

// Baseline Smaller (from /usr/include/CGAL/Nef_3/K3_tree.h)
struct Baseline_Smaller {
    int coord;
    Baseline_Smaller(int c) : coord(c) {}
    bool operator()(const Vertex_handle& v1, const Vertex_handle& v2) const {
        switch(coord) {
            case 0: return CGAL::compare_x(v1->point(), v2->point()) == CGAL::SMALLER;
            case 1: return CGAL::compare_y(v1->point(), v2->point()) == CGAL::SMALLER;
            case 2: return CGAL::compare_z(v1->point(), v2->point()) == CGAL::SMALLER;
            default: return false;
        }
    }
};

static Point_3 find_median_point_baseline(Vertex_list& V, int coord) {
  Baseline_Smaller smaller(coord);
  Vertex_iterator begin = V.begin();
  Vertex_iterator median = begin + V.size()/2;
  std::nth_element(begin, median, V.end(), smaller);
  Vertex_iterator prev = std::prev(median);
  std::nth_element(begin, prev, median, smaller);

  return CGAL::midpoint((*median)->point(), (*prev)->point());
}

// Contender Smaller (provided by user)
template <typename Compare>
struct Contender_Smaller {
    Compare compare;
    bool operator()(const Vertex_handle& a, const Vertex_handle& b) const {
        return compare(a->point(), b->point()) == CGAL::SMALLER;
    }
};

template <typename Smaller>
static Point_3 find_median_point_contender_impl(Vertex_list& V) {
    Smaller smaller;
    auto upper = std::next(V.begin(), V.size() / 2);
    std::nth_element(V.begin(), upper, V.end(), smaller);
    auto lower = std::max_element(V.begin(), upper, smaller);

    return CGAL::midpoint((*lower)->point(), (*upper)->point());
}

static Point_3 find_median_point_contender(Vertex_list& V, int coord) {
    typedef Contender_Smaller<Kernel::Compare_x_3> Smaller_x_3;
    typedef Contender_Smaller<Kernel::Compare_y_3> Smaller_y_3;
    typedef Contender_Smaller<Kernel::Compare_z_3> Smaller_z_3;

    switch(coord) {
        case 0: return find_median_point_contender_impl<Smaller_x_3>(V);
        case 1: return find_median_point_contender_impl<Smaller_y_3>(V);
        case 2: return find_median_point_contender_impl<Smaller_z_3>(V);
        default:
            return Point_3(0, 0, 0);
    }
}

static void BM_Baseline(benchmark::State& state) {
  int size = 10000;
  CGAL::Random rnd(1337);
  CGAL::Random_points_in_cube_3<Point_3> gen(100.0, rnd);
  std::vector<Vertex> vertices;
  for(int i=0; i<size; ++i) {
      vertices.emplace_back(*gen++);
  }

  for (auto _ : state) {
    state.PauseTiming();
    Vertex_list v_list;
    v_list.reserve(size);
    for(int i=0; i<size; ++i) v_list.push_back(&vertices[i]);
    state.ResumeTiming();
    benchmark::DoNotOptimize(find_median_point_baseline(v_list, 0));
    benchmark::DoNotOptimize(find_median_point_baseline(v_list, 1));
    benchmark::DoNotOptimize(find_median_point_baseline(v_list, 2));
  }
}

static void BM_Contender(benchmark::State& state) {
  int size = 10000;
  CGAL::Random rnd(1337);
  CGAL::Random_points_in_cube_3<Point_3> gen(100.0, rnd);
  std::vector<Vertex> vertices;
  for(int i=0; i<size; ++i) {
      vertices.emplace_back(*gen++);
  }

  for (auto _ : state) {
    state.PauseTiming();
    Vertex_list v_list;
    v_list.reserve(size);
    for(int i=0; i<size; ++i) v_list.push_back(&vertices[i]);
    state.ResumeTiming();
    benchmark::DoNotOptimize(find_median_point_contender(v_list, 0));
    benchmark::DoNotOptimize(find_median_point_contender(v_list, 1));
    benchmark::DoNotOptimize(find_median_point_contender(v_list, 2));
  }
}

BENCHMARK(BM_Baseline);

BENCHMARK(BM_Contender);

BENCHMARK_MAIN();
