#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Nef_S2/SM_decorator.h>
#include <CGAL/Nef_3/SNC_sphere_map.h>
#include <CGAL/Nef_3/SNC_structure.h>
#include <CGAL/Nef_3/SNC_indexed_items.h>
#include <CGAL/Nef_3/bounded_side_3.h>
#include <CGAL/Nef_3/SNC_intersection.h>
#include <CGAL/Random.h>
#include <CGAL/point_generators_3.h>
#include <benchmark/benchmark.h>
#include <vector>
#include <variant>
#include <memory>
#include <iostream>

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef CGAL::SNC_indexed_items Items;
typedef bool Mark;
typedef CGAL::SNC_structure<Kernel, Items, Mark> SNC_structure;

struct Traits {
    typedef ::Kernel Kernel;
    typedef ::Items Items;
    typedef ::Mark Mark;
    typedef ::SNC_structure SNC_structure;

    typedef Kernel::Point_3 Point_3;
    typedef Kernel::Segment_3 Segment_3;
    typedef Kernel::Ray_3 Ray_3;
    typedef Kernel::Plane_3 Plane_3;
    typedef Kernel::Vector_3 Vector_3;

    typedef SNC_structure::Vertex_handle Vertex_handle;
    typedef SNC_structure::Halfedge_handle Halfedge_handle;
    typedef SNC_structure::Halffacet_handle Halffacet_handle;
    typedef SNC_structure::SHalfedge_handle SHalfedge_handle;

    typedef SNC_structure::Halfedge_const_handle Halfedge_const_handle;
    typedef SNC_structure::Halffacet_const_handle Halffacet_const_handle;

    struct CaseData {
        Point_3 p1s, p1t, p2s, p2t;
        Plane_3 pl;
        CaseData(Point_3 p1s, Point_3 p1t, Point_3 p2s, Point_3 p2t, Plane_3 pl = Plane_3(0,0,1,0))
            : p1s(p1s), p1t(p1t), p2s(p2s), p2t(p2t), pl(pl) {}
    };

    static std::vector<CaseData> create_cases_data() {
        std::vector<CaseData> cases;
        cases.emplace_back(Point_3(0,0,0), Point_3(2,0,0), Point_3(2,0,0), Point_3(2,2,0));
        cases.emplace_back(Point_3(0,0,0), Point_3(2,0,0), Point_3(1,1,1), Point_3(1,-1,1));
        cases.emplace_back(Point_3(0,0,0), Point_3(1,0,0), Point_3(0.5, 0, 0), Point_3(0.5, 1, 0));
        cases.emplace_back(Point_3(0,0,0), Point_3(1,0,0), Point_3(0.5, 1, 0), Point_3(0.5, 0, 0));
        cases.emplace_back(Point_3(0,0,0), Point_3(1,0,0), Point_3(0, -1, 0), Point_3(0, 1, 0));
        cases.emplace_back(Point_3(0,0,0), Point_3(1,0,0), Point_3(0,1,0), Point_3(1,1,0));
        cases.emplace_back(Point_3(1,0,0), Point_3(2,0,0), Point_3(0,-1,0), Point_3(0,1,0));
        cases.emplace_back(Point_3(-1,0,0), Point_3(1,0,0), Point_3(0,1,0), Point_3(0,2,0));
        cases.emplace_back(Point_3(-1,0,0), Point_3(1,0,0), Point_3(0,-2,0), Point_3(0,-1,0));
        cases.emplace_back(Point_3(0,0,0), Point_3(1,0,0), Point_3(2,-1,0), Point_3(2,1,0));
        cases.emplace_back(Point_3(0,0,0), Point_3(2,0,0), Point_3(1,-1,0), Point_3(1,1,0));
        cases.emplace_back(Point_3(0,0,10), Point_3(0,0,-10), Point_3(0,0,0), Point_3(1,1,1));
        cases.emplace_back(Point_3(0,0,10), Point_3(0,0,-10), Point_3(200,200,0), Point_3(1,1,1));
        cases.emplace_back(Point_3(0,0,10), Point_3(0,0,-10), Point_3(0,0,5), Point_3(1,1,1));
        cases.emplace_back(Point_3(0,0,10), Point_3(0,0,-10), Point_3(100,100,0), Point_3(1,1,1));
        cases.emplace_back(Point_3(0,0,0), Point_3(0,0,10), Point_3(1,1,1), Point_3(2,2,2));
        cases.emplace_back(Point_3(0,0,10), Point_3(0,0,0), Point_3(1,1,1), Point_3(2,2,2));
        cases.emplace_back(Point_3(200,200,10), Point_3(200,200,-10), Point_3(1,1,1), Point_3(2,2,2));
        return cases;
    }

    struct Case {
        std::unique_ptr<SNC_structure> snc;
        Point_3 p1s, p1t, p2s, p2t;
        Segment_3 s1, s2;
        Ray_3 r1;
        Halfedge_const_handle e1, e2;
        Halffacet_const_handle f1;

        Case(const CaseData& d) : snc(std::make_unique<SNC_structure>()), p1s(d.p1s), p1t(d.p1t), p2s(d.p2s), p2t(d.p2t) {
            s1 = Segment_3(p1s, p1t);
            s2 = Segment_3(p2s, p2t);
            r1 = Ray_3(p1s, p1t);

            auto v1s_h = snc->new_vertex(p1s);
            auto v1t_h = snc->new_vertex(p1t);
            auto v2s_h = snc->new_vertex(p2s);
            auto v2t_h = snc->new_vertex(p2t);

            auto make_edge_pair = [&](Vertex_handle v_src, Vertex_handle v_tgt) {
                CGAL::SM_decorator<SNC_structure::Sphere_map> SM_src(&*v_src), SM_tgt(&*v_tgt);
                SNC_structure::SVertex_handle e_src = SM_src.new_svertex();
                SNC_structure::SVertex_handle e_tgt = SM_tgt.new_svertex();
                e_src->twin() = e_tgt; e_tgt->twin() = e_src;
                e_src->center_vertex() = v_src; e_tgt->center_vertex() = v_tgt;
                return e_src;
            };

            e1 = make_edge_pair(v1s_h, v1t_h);
            e2 = make_edge_pair(v2s_h, v2t_h);
            auto f1_h = snc->new_halffacet_pair(d.pl);
            f1 = f1_h;

            Vertex_handle tv1 = snc->new_vertex(Point_3(100, 100, 0));
            Vertex_handle tv2 = snc->new_vertex(Point_3(-100, 100, 0));
            Vertex_handle tv3 = snc->new_vertex(Point_3(0, -100, 0));
            auto he1 = make_edge_pair(tv1, tv2);
            auto he2 = make_edge_pair(tv2, tv3);
            auto he3 = make_edge_pair(tv3, tv1);
            auto she1 = snc->new_shalfedge_only();
            auto she2 = snc->new_shalfedge_only();
            auto she3 = snc->new_shalfedge_only();
            she1->source() = he1; she1->facet() = f1_h; she1->next() = she2; she1->prev() = she3;
            she2->source() = he2; she2->facet() = f1_h; she2->next() = she3; she2->prev() = she1;
            she3->source() = he3; she3->facet() = f1_h; she3->next() = she1; she3->prev() = she2;
            f1_h->boundary_entry_objects().push_back(CGAL::make_object(she1));
        }
    };
};

namespace Baseline {
    typedef ::CGAL::SNC_intersection<SNC_structure> SNC_intersection;
    inline bool intersect_ee(Traits::Halfedge_const_handle e1, Traits::Halfedge_const_handle e2, Traits::Point_3& p) { return SNC_intersection::does_intersect_internally(Traits::Segment_3(e1->source()->point(), e1->twin()->source()->point()), Traits::Segment_3(e2->source()->point(), e2->twin()->source()->point()), p); }
    inline bool intersect_ef(Traits::Halfedge_const_handle e1, Traits::Halffacet_const_handle f2, Traits::Point_3& p) { return SNC_intersection::does_intersect_internally(Traits::Segment_3(e1->source()->point(), e1->twin()->source()->point()), f2, p); }
    inline bool intersect_se(const Traits::Segment_3& s1, Traits::Halfedge_const_handle e2, Traits::Point_3& p) { return SNC_intersection::does_intersect_internally(s1, Traits::Segment_3(e2->source()->point(), e2->twin()->source()->point()), p); }
    inline bool intersect_sf(const Traits::Segment_3& s1, Traits::Halffacet_const_handle f2, Traits::Point_3& p) { return SNC_intersection::does_intersect_internally(s1, f2, p); }
    inline bool intersect_re(const Traits::Ray_3& r1, Traits::Halfedge_const_handle e2, Traits::Point_3& p) { return SNC_intersection::does_intersect_internally(r1, Traits::Segment_3(e2->source()->point(), e2->twin()->source()->point()), p); }
    inline bool intersect_rf(const Traits::Ray_3& r1, Traits::Halffacet_const_handle f2, Traits::Point_3& p) { return SNC_intersection::does_intersect_internally(r1, f2, p); }
}

namespace Contender {
    namespace CGAL {
        using namespace ::CGAL;
    }
    #undef CGAL_BOUNDED_SIDE_3_H
    #include "bounded_side_3.h"
    #undef CGAL_SNC_INTERSECTION_H
    #include "SNC_intersection.h"

    typedef Contender::CGAL::SNC_intersection<SNC_structure> SNC_intersection;
    inline bool intersect_ee(Traits::Halfedge_const_handle e1, Traits::Halfedge_const_handle e2, Traits::Point_3& p) { return SNC_intersection::does_intersect_internally(e1, e2, p); }
    inline bool intersect_ef(Traits::Halfedge_const_handle e1, Traits::Halffacet_const_handle f2, Traits::Point_3& p) { return SNC_intersection::does_intersect_internally(e1, f2, p); }
    inline bool intersect_se(const Traits::Segment_3& s1, Traits::Halfedge_const_handle e2, Traits::Point_3& p) { return SNC_intersection::does_intersect_internally(s1, e2, p); }
    inline bool intersect_sf(const Traits::Segment_3& s1, Traits::Halffacet_const_handle f2, Traits::Point_3& p) { return SNC_intersection::does_intersect_internally(s1, f2, p); }
    inline bool intersect_re(const Traits::Ray_3& r1, Traits::Halfedge_const_handle e2, Traits::Point_3& p) { return SNC_intersection::does_intersect_internally(r1, e2, p); }
    inline bool intersect_rf(const Traits::Ray_3& r1, Traits::Halffacet_const_handle f2, Traits::Point_3& p) { return SNC_intersection::does_intersect_internally(r1, f2, p); }
}

void run_functional_tests() {
    auto data = Traits::create_cases_data();
    for (size_t i = 0; i < data.size(); ++i) {
        Traits::Case c(data[i]);
        Traits::Point_3 p1, p2;
        if (Baseline::intersect_ee(c.e1, c.e2, p1) != Contender::intersect_ee(c.e1, c.e2, p2)) { std::cerr << "Fail intersect_ee " << i << std::endl; exit(1); }
        if (Baseline::intersect_ef(c.e1, c.f1, p1) != Contender::intersect_ef(c.e1, c.f1, p2)) { std::cerr << "Fail intersect_ef " << i << std::endl; exit(1); }
        if (Baseline::intersect_se(c.s1, c.e2, p1) != Contender::intersect_se(c.s1, c.e2, p2)) { std::cerr << "Fail intersect_se " << i << std::endl; exit(1); }
        if (Baseline::intersect_sf(c.s1, c.f1, p1) != Contender::intersect_sf(c.s1, c.f1, p2)) { std::cerr << "Fail intersect_sf " << i << std::endl; exit(1); }
        if (Baseline::intersect_re(c.r1, c.e2, p1) != Contender::intersect_re(c.r1, c.e2, p2)) { std::cerr << "Fail intersect_re " << i << std::endl; exit(1); }
        if (Baseline::intersect_rf(c.r1, c.f1, p1) != Contender::intersect_rf(c.r1, c.f1, p2)) { std::cerr << "Fail intersect_rf " << i << std::endl; exit(1); }
    }
    std::cout << "All functional tests passed!" << std::endl;
}

#define BENCHMARK_PAIR(Func, CallBaseline, CallContender) \
static void BM_##Func##_Baseline(benchmark::State& state) { \
    auto data = Traits::create_cases_data(); \
    std::vector<std::unique_ptr<Traits::Case>> cases; \
    for(const auto& d : data) cases.emplace_back(std::make_unique<Traits::Case>(d)); \
    Traits::Point_3 p; \
    for (auto _ : state) { \
        for (const auto& c_ptr : cases) { \
            auto& c = *c_ptr; \
            benchmark::DoNotOptimize(Baseline::CallBaseline); \
        } \
    } \
} \
BENCHMARK(BM_##Func##_Baseline); \
static void BM_##Func##_Contender(benchmark::State& state) { \
    auto data = Traits::create_cases_data(); \
    std::vector<std::unique_ptr<Traits::Case>> cases; \
    for(const auto& d : data) cases.emplace_back(std::make_unique<Traits::Case>(d)); \
    Traits::Point_3 p; \
    for (auto _ : state) { \
        for (const auto& c_ptr : cases) { \
            auto& c = *c_ptr; \
            benchmark::DoNotOptimize(Contender::CallContender); \
        } \
    } \
} \
BENCHMARK(BM_##Func##_Contender);

BENCHMARK_PAIR(Intersect_EE, intersect_ee(c.e1, c.e2, p), intersect_ee(c.e1, c.e2, p))
BENCHMARK_PAIR(Intersect_EF, intersect_ef(c.e1, c.f1, p), intersect_ef(c.e1, c.f1, p))
BENCHMARK_PAIR(Intersect_SE, intersect_se(c.s1, c.e2, p), intersect_se(c.s1, c.e2, p))
BENCHMARK_PAIR(Intersect_SF, intersect_sf(c.s1, c.f1, p), intersect_sf(c.s1, c.f1, p))
BENCHMARK_PAIR(Intersect_RE, intersect_re(c.r1, c.e2, p), intersect_re(c.r1, c.e2, p))
BENCHMARK_PAIR(Intersect_RF, intersect_rf(c.r1, c.f1, p), intersect_rf(c.r1, c.f1, p))

int main(int argc, char** argv) {
    run_functional_tests();
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
