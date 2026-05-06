#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/algorithm.h>
#include <benchmark/benchmark.h>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>

typedef CGAL::Exact_predicates_exact_constructions_kernel Bench_Kernel;
typedef CGAL::SNC_indexed_items Bench_Items;
typedef bool Bench_Mark;
typedef CGAL::Nef_polyhedron_3<Bench_Kernel, Bench_Items, Bench_Mark> Bench_Nef;
typedef Bench_Nef::SNC_structure Bench_SNC;

struct Bench_Case {
    std::unique_ptr<Bench_Nef> nef;
    std::unique_ptr<Bench_Nef> nef2;
    Bench_SNC::Halfedge_const_handle e;
    Bench_SNC::Halffacet_const_handle f;
    Bench_Kernel::Point_3 p;

    Bench_Case(int type = 0) {
        std::stringstream ss;
        ss << "OFF\n8 6 0\n"
           << "-1 -1 -1\n" << " 1 -1 -1\n" << " 1  1 -1\n" << "-1  1 -1\n"
           << "-1 -1  1\n" << " 1 -1  1\n" << " 1  1  1\n" << "-1  1  1\n"
           << "4 0 1 2 3\n" << "4 4 7 6 5\n" << "4 0 4 5 1\n" << "4 1 5 6 2\n" << "4 2 6 7 3\n" << "4 3 7 4 0\n";
        CGAL::Polyhedron_3<Bench_Kernel> P;
        ss >> P;
        if (type == 0 || type == 3) {
            nef = std::make_unique<Bench_Nef>(P);
            const Bench_SNC* snc = nef->sncp();
            e = snc->halfedges_begin();
            f = snc->halffacets_begin();
            p = CGAL::midpoint(e->source()->point(), e->twin()->source()->point());
        } else if (type == 1 || type == 2 || type == 4) {
            Bench_Kernel::Segment_3 s(Bench_Kernel::Point_3(0,0,-1), Bench_Kernel::Point_3(0,0,1));
            nef = std::make_unique<Bench_Nef>(s);
            e = nef->sncp()->halfedges_begin();
            nef2 = std::make_unique<Bench_Nef>(P);
            f = nef2->sncp()->halffacets_begin();
            p = Bench_Kernel::Point_3(0,0,0);
        } else if (type == 5) {
            // Case 5: Crossing segment to hit oriented_side logic
            Bench_Kernel::Segment_3 s(Bench_Kernel::Point_3(0,0,0), Bench_Kernel::Point_3(2,0,0));
            nef = std::make_unique<Bench_Nef>(s);
            e = nef->sncp()->halfedges_begin();
            nef2 = std::make_unique<Bench_Nef>(P);
            // Find facet at x=1. Plane is x-1=0? or 1-x=0.
            // In CGAL Nef3, facets of a cube.
            f = nef2->sncp()->halffacets_begin();
            while (f != nef2->sncp()->halffacets_end() && f->plane().oriented_side(Bench_Kernel::Point_3(2,0,0)) != CGAL::ON_NEGATIVE_SIDE) {
                ++f;
            }
            if (f == nef2->sncp()->halffacets_end()) f = nef2->sncp()->halffacets_begin();
            p = Bench_Kernel::Point_3(1,0,0);
        }
    }
};

struct Bench_Selection {
    int type;
    Bench_Selection(int t = 0) : type(t) {}
    Bench_Mark operator()(Bench_Mark m1, Bench_Mark m2) const { 
        if (type == 2) return m1 ^ m2;
        if (type == 4) return true;
        return m1 || m2; 
    }
    Bench_Mark operator()(Bench_Mark m1, Bench_Mark m2, bool) const { 
        if (type == 2) return m1 ^ m2;
        if (type == 4) return true;
        return m1 || m2; 
    }
};

namespace Baseline {
    typedef ::CGAL::SNC_constructor<Bench_Items, Bench_SNC> SNC_constructor;
    typedef ::CGAL::ID_support_handler<Bench_Items, ::CGAL::SNC_decorator<Bench_SNC>> Association;
    inline void run(Bench_Case& c, int type = 0) {
        Bench_SNC output_snc;
        SNC_constructor constructor(output_snc);
        Association assoc;
        Bench_Selection sel(type);
        bool inv = (type == 3);
        constructor.create_edge_facet_overlay(c.e, c.f, c.p, sel, inv, assoc);
    }
}

namespace Contender {
    namespace CGAL {
        using namespace ::CGAL;
    }
    #undef CGAL_SNC_CONSTRUCTOR_H
    #include "SNC_constructor.h"

    typedef Contender::CGAL::SNC_constructor<Bench_Items, Bench_SNC> SNC_constructor;
    typedef ::CGAL::ID_support_handler<Bench_Items, ::CGAL::SNC_decorator<Bench_SNC>> Association;

    inline void run(Bench_Case& c, int type = 0) {
        Bench_SNC output_snc;
        SNC_constructor constructor(output_snc);
        Association assoc;
        Bench_Selection sel(type);
        bool inv = (type == 3);
        constructor.create_edge_facet_overlay(c.e, c.f, c.p, sel, inv, assoc);
    }
}

static void BM_CreateEdgeFacetOverlay_Baseline(benchmark::State& state) {
    Bench_Case c(state.range(0));
    for (auto _ : state) {
        Baseline::run(c, state.range(0));
    }
}
BENCHMARK(BM_CreateEdgeFacetOverlay_Baseline)->DenseRange(0, 5);

static void BM_CreateEdgeFacetOverlay_Contender(benchmark::State& state) {
    Bench_Case c(state.range(0));
    for (auto _ : state) {
        Contender::run(c, state.range(0));
    }
}
BENCHMARK(BM_CreateEdgeFacetOverlay_Contender)->DenseRange(0, 5);

#ifdef CODE_COVERAGE
void run_coverage_tests_using_bench_cases() {
    std::cout << "Running coverage tests..." << std::endl;
    for (int i = 0; i <= 5; ++i) {
        std::cout << "Case " << i << "..." << std::endl;
        Bench_Case c(i);
        Contender::run(c, i);
    }
    std::cout << "Coverage tests complete." << std::endl;
}

int main(int argc, char** argv) {
    run_coverage_tests_using_bench_cases();
    return 0;
}
#else
int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
#endif
