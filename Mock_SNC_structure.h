#ifndef MOCK_SNC_H
#define MOCK_SNC_H

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Random.h>
#include <CGAL/point_generators_3.h>
#include <vector>
#include <deque>
#include <variant>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point_3;

// Forward declarations
struct Vertex;
struct Halfedge;
struct Halffacet;
struct SVertex;
struct SHalfedge;

typedef Vertex* Vertex_handle;
typedef Halfedge* Halfedge_handle;
typedef Halffacet* Halffacet_handle;
typedef SVertex* SVertex_handle;
typedef SHalfedge* SHalfedge_handle;

struct Vertex {
  Point_3 p;
  Vertex(const Point_3& p_) : p(p_) {}
  const Point_3& point() const { return p; }
};

struct Halfedge {
  Vertex_handle s;
  Halfedge_handle t;
  bool twin_flag;
  Halfedge() : s(nullptr), t(nullptr), twin_flag(false) {}
  Vertex_handle source() const { return s; }
  Halfedge_handle twin() const { return t; }
  bool is_twin() const { return twin_flag; }
};

struct SVertex {
  Vertex_handle v;
  SVertex(Vertex_handle v_) : v(v_) {}
  Vertex_handle center_vertex() const { return v; }
};

struct SHalfedge {
  SVertex_handle sv;
  SHalfedge_handle next_ptr;
  SHalfedge(SVertex_handle sv_) : sv(sv_), next_ptr(nullptr) {}
  SVertex_handle source() const { return sv; }
  SHalfedge_handle next() const { return next_ptr; }
};

struct Mock_circulator {
  SHalfedge_handle ptr;
  Mock_circulator(SHalfedge_handle p) : ptr(p) {}
  SHalfedge_handle operator->() const { return ptr; }
  SHalfedge& operator*() const { return *ptr; }
  bool operator==(const Mock_circulator& other) const { return ptr == other.ptr; }
  bool operator!=(const Mock_circulator& other) const { return ptr != other.ptr; }
  Mock_circulator& operator++() { ptr = ptr->next_ptr; return *this; }
};

struct Mock_facet_cycle_iterator {
  typedef std::deque<SHalfedge_handle>::iterator Base;
  Base it;
  Mock_facet_cycle_iterator(Base it_) : it(it_) {}
  bool is_shalfedge() const { return true; }
  SHalfedge_handle operator*() const { return *it; }
  SHalfedge_handle operator->() const { return *it; }
  bool operator==(const Mock_facet_cycle_iterator& other) const { return it == other.it; }
  bool operator!=(const Mock_facet_cycle_iterator& other) const { return it != other.it; }
  Mock_facet_cycle_iterator& operator++() { ++it; return *this; }
  operator SHalfedge_handle() const { return *it; }
};

struct Halffacet {
  bool twin_flag;
  std::deque<SHalfedge_handle> cycles;
  Halffacet() : twin_flag(false) {}
  bool is_twin() const { return twin_flag; }
  typedef Mock_facet_cycle_iterator Halffacet_cycle_iterator;
  Halffacet_cycle_iterator facet_cycles_begin() { return Halffacet_cycle_iterator(cycles.begin()); }
  Halffacet_cycle_iterator facet_cycles_end() { return Halffacet_cycle_iterator(cycles.end()); }
};

template<typename T, typename Handle>
struct Mock_iterator {
  typedef typename std::deque<T>::iterator Base;
  Base it;
  Mock_iterator() {}
  Mock_iterator(const Base& it_) : it(it_) {}
  T& operator*() const { return *it; }
  T* operator->() const { return &(*it); }
  Mock_iterator& operator++() { ++it; return *this; }
  Mock_iterator operator++(int) { Mock_iterator tmp = *this; ++it; return tmp; }
  bool operator==(const Mock_iterator& other) const { return it == other.it; }
  bool operator!=(const Mock_iterator& other) const { return it != other.it; }
  operator Handle() const { return &(*it); }
};

struct Mock_SNC_structure {
  typedef ::Kernel Kernel;
  struct Infi_box {};
  typedef std::variant<Vertex_handle, Halfedge_handle, Halffacet_handle> Object_handle;

  std::deque<Vertex> vertices;
  std::deque<Halfedge> halfedges;
  std::deque<Halffacet> halffacets;
  std::deque<SVertex> svertices;
  std::deque<SHalfedge> shalfedges;

  typedef Mock_iterator<Vertex, Vertex_handle> Vertex_iterator;
  typedef Mock_iterator<Halfedge, Halfedge_handle> Halfedge_iterator;
  typedef Mock_iterator<Halffacet, Halffacet_handle> Halffacet_iterator;

  Vertex_iterator vertices_begin() { return Vertex_iterator(vertices.begin()); }
  Vertex_iterator vertices_end() { return Vertex_iterator(vertices.end()); }
  Halfedge_iterator halfedges_begin() { return Halfedge_iterator(halfedges.begin()); }
  Halfedge_iterator halfedges_end() { return Halfedge_iterator(halfedges.end()); }
  Halffacet_iterator halffacets_begin() { return Halffacet_iterator(halffacets.begin()); }
  Halffacet_iterator halffacets_end() { return Halffacet_iterator(halffacets.end()); }

  size_t number_of_vertices() const { return vertices.size(); }
  size_t number_of_halfedges() const { return halfedges.size(); }
  size_t number_of_halffacets() const { return halffacets.size(); }
};

struct Mock_decorator_traits {
  typedef ::Vertex_handle Vertex_handle;
  typedef ::Halfedge_handle Halfedge_handle;
  typedef ::Halffacet_handle Halffacet_handle;
  typedef ::SHalfedge_handle SHalfedge_handle;
  typedef Halffacet::Halffacet_cycle_iterator Halffacet_cycle_iterator;
  typedef Mock_circulator SHalfedge_around_facet_circulator;
};

struct Mock_decorator {
  typedef Mock_SNC_structure SNC_structure;
  typedef Mock_decorator_traits Decorator_traits;
  typedef ::Kernel Kernel;
};

inline void populate(Mock_SNC_structure& snc, int n) {
  CGAL::Random rnd(42);
  CGAL::Random_points_in_cube_3<Point_3> gen(100.0, rnd);
  for(int i=0; i<n; ++i) {
    Point_3 p[3];
    p[0] = *gen++; p[1] = *gen++; p[2] = *gen++;
    Vertex_handle v[3];
    for(int j=0; j<3; ++j) {
      snc.vertices.emplace_back(p[j]);
      v[j] = &snc.vertices.back();
    }
    auto add_edge = [&](Vertex_handle va, Vertex_handle vb) {
        snc.halfedges.emplace_back();
        Halfedge_handle e1 = &snc.halfedges.back();
        snc.halfedges.emplace_back();
        Halfedge_handle e2 = &snc.halfedges.back();
        e1->s = va; e1->t = e2; e2->s = vb; e2->t = e1; e2->twin_flag = true;
        return e1;
    };
    add_edge(v[0], v[1]); add_edge(v[1], v[2]); add_edge(v[2], v[0]);
    SVertex_handle sv[3];
    for(int j=0; j<3; ++j) {
      snc.svertices.emplace_back(v[j]);
      sv[j] = &snc.svertices.back();
    }
    SHalfedge_handle sh[3];
    for(int j=0; j<3; ++j) {
      snc.shalfedges.emplace_back(sv[j]);
      sh[j] = &snc.shalfedges.back();
    }
    for(int j=0; j<3; ++j) sh[j]->next_ptr = sh[(j+1)%3];
    snc.halffacets.emplace_back();
    Halffacet_handle f = &snc.halffacets.back();
    f->cycles.push_back(sh[0]);
    snc.halffacets.emplace_back();
    snc.halffacets.back().twin_flag = true;
  }
}

#endif // MOCK_SNC_H
