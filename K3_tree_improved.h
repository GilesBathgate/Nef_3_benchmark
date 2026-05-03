#ifndef CGAL_NEF_K3_TREE_IMPROVED_H
#define CGAL_NEF_K3_TREE_IMPROVED_H

#include <CGAL/license/Nef_3.h>
#include <CGAL/basic.h>
#include <CGAL/Nef_3/SNC_iteration.h>
#include <CGAL/enum.h>
#include <CGAL/tags.h>
#include <boost/container/deque.hpp>
#include <algorithm>
#include <utility>
#include <ostream>

namespace Improved {

template <typename Kernel, typename Coordinate>
class Compare_points {

  typedef typename Kernel::Point_3  Point_3;
 public:
  Compare_points(Coordinate c) : coord(c) {
    CGAL_assertion( c >= 0 && c <=2);
  }
  CGAL::Comparison_result operator()(const Point_3& p1, const Point_3& p2) {
    switch(coord) {
    case 0:
      CGAL_NEF_TRACEN("compare_x " << p1 << ", " << p2 << "=" << (int) CGAL::compare_x(p1, p2));
      return CGAL::compare_x(p1, p2);
    case 1:
      CGAL_NEF_TRACEN("compare_y " << p1 << ", " << p2 << "=" << (int) CGAL::compare_y(p1, p2));
      return CGAL::compare_y(p1, p2);
    case 2:
      CGAL_NEF_TRACEN("compare_z " << p1 << ", " << p2 << "=" << (int) CGAL::compare_z(p1, p2));
      return CGAL::compare_z(p1, p2);
    default: CGAL_error();
    }
    return CGAL::EQUAL;
  }
private:
  Coordinate coord;
};
  
template <class SNC_decorator>
class Side_of_plane {
  
  typedef typename SNC_decorator::Decorator_traits Decorator_traits;

  typedef typename Decorator_traits::Vertex_handle Vertex_handle;
  typedef typename Decorator_traits::Halfedge_handle Halfedge_handle;
  typedef typename Decorator_traits::Halffacet_handle Halffacet_handle;

  typedef typename Decorator_traits::Halffacet_cycle_iterator
    Halffacet_cycle_iterator;
  typedef typename Decorator_traits::SHalfedge_around_facet_circulator
    SHalfedge_around_facet_circulator;
  typedef typename Decorator_traits::SHalfedge_handle SHalfedge_handle;

  typedef typename SNC_decorator::Kernel Kernel;
  typedef typename Kernel::Point_3 Point_3;
  typedef Compare_points<Kernel, int> Compare;
  typedef CGAL::Oriented_side Oriented_side;
  typedef CGAL::Comparison_result Comparison_result;

  struct Entry {
    Oriented_side side;
    int generation;
  };

public:
  Side_of_plane(const Point_3& p, int c) :
    side_map(Entry()), generation(1), coord(c), pop(p) {}
  Side_of_plane(std::size_t num_vertices) :
    side_map(Entry(), num_vertices), generation(1) {}
  void next(const Point_3& p, int c) { pop = p; coord = c; ++generation; }
  void set_side(Vertex_handle v, Oriented_side side) {
    side_map[v] = {side, generation};
  }
  Oriented_side operator()(Vertex_handle v){
    auto& entry = side_map[v];
    if(entry.generation != generation) {
    Compare compare(coord);
      Comparison_result cr = compare(v->point(), pop);
      Oriented_side side = cr == CGAL::LARGER ? CGAL::ON_POSITIVE_SIDE :
                          cr == CGAL::SMALLER ? CGAL::ON_NEGATIVE_SIDE :
                          CGAL::ON_ORIENTED_BOUNDARY;
      entry = {side, generation};
  }
    return entry.side;
}
  Oriented_side operator()(Halfedge_handle e){
  Vertex_handle v = e->source();
  Vertex_handle vt = e->twin()->source();

  Oriented_side src_side = (*this) (v);
  Oriented_side tgt_side = (*this) (vt);
  if( src_side == tgt_side)
    return src_side;
  if( src_side == CGAL::ON_ORIENTED_BOUNDARY)
    return tgt_side;
  if( tgt_side == CGAL::ON_ORIENTED_BOUNDARY)
    return src_side;
  return CGAL::ON_ORIENTED_BOUNDARY;
}
  Oriented_side operator()(Halffacet_handle f){

  CGAL_assertion(f->facet_cycles_begin() != f->facet_cycles_end());

  Halffacet_cycle_iterator fc(f->facet_cycles_begin());
  CGAL_assertion(fc.is_shalfedge());

  SHalfedge_handle e(fc);
  SHalfedge_around_facet_circulator sc(e), send(sc);

  Oriented_side facet_side = CGAL::ON_ORIENTED_BOUNDARY;

  do {
    Vertex_handle v = sc->source()->center_vertex();
    Oriented_side point_side = (*this)(v);

    if (point_side != CGAL::ON_ORIENTED_BOUNDARY) {
      if (facet_side == CGAL::ON_ORIENTED_BOUNDARY) {
        facet_side = point_side;
      } else if (facet_side != point_side) {
        return CGAL::ON_ORIENTED_BOUNDARY;
      }
    }
    ++sc;
  } while (sc != send);

  return facet_side;
}
private:
  CGAL::Unique_hash_map<Vertex_handle,Entry> side_map;
  int generation;
  int coord;
  Point_3 pop;
};

template <class Traits>
class K3_tree
{
public:
  typedef typename Traits::Vertex_handle Vertex_handle;
  typedef typename Traits::Vertex_list Vertex_list;
  typedef typename Vertex_list::iterator Vertex_iterator;
  typedef typename Traits::Halfedge_handle Halfedge_handle;
  typedef typename Traits::Halffacet_handle Halffacet_handle;
  typedef typename Traits::Halfedge_list Halfedge_list;
  typedef typename Traits::Halffacet_list Halffacet_list;
  typedef typename Traits::Point_3 Point_3;
  typedef typename Traits::Plane_3 Plane_3;
  typedef typename Traits::Vector_3 Vector_3;
  typedef typename Traits::Bounding_box_3 Bounding_box_3;
  typedef typename Improved::Side_of_plane<typename Traits::SNC_decorator> Side_of_plane;
  typedef typename Traits::Kernel Kernel;

private:
  struct Leaf {
    Vertex_list vertex_list;
    Halfedge_list edge_list;
    Halffacet_list facet_list;
    void shrink_to_fit() {
      vertex_list.shrink_to_fit();
      edge_list.shrink_to_fit();
      facet_list.shrink_to_fit();
    }
  };

public:

  class Node  {
    friend class K3_tree<Traits>;
  public:
      typedef Node* Node_handle;
      typedef std::unique_ptr<Leaf> Leaf_handle;

    Node(Leaf&& l) :
      left_node(nullptr), right_node(nullptr), leaf(std::make_unique<Leaf>(std::move(l)))
    {
      leaf->shrink_to_fit();
    }

    Node(Node_handle l, Node_handle r, const Plane_3& pl) :
      left_node(l), right_node(r), leaf(nullptr), splitting_plane(pl)
    {
    }

    bool is_leaf() const { return (left_node == nullptr && right_node == nullptr); }
  private:
    Node_handle left_node;
    Node_handle right_node;
    Leaf_handle leaf;
    Plane_3 splitting_plane;
  };

  typedef boost::container::deque<Node> Node_range;
  typedef Node* Node_handle;

private:
  Node_handle root; Node_range nodes;
  int max_depth; Bounding_box_3 bounding_box;
  Side_of_plane side_of_plane;

public:
  template<typename SNC_structure>
  K3_tree(SNC_structure* W)
    : side_of_plane(W->number_of_vertices())
  {
    Vertex_list vertices; vertices.reserve(W->number_of_vertices());
    typename SNC_structure::Vertex_iterator v;
    CGAL_forall_vertices(v, *W) vertices.push_back(v);
    Halfedge_list edges; edges.reserve(W->number_of_halfedges());
    typename SNC_structure::Halfedge_iterator e;
    CGAL_forall_edges(e, *W) edges.push_back(e);
    Halffacet_list facets; facets.reserve(W->number_of_halffacets());
    typename SNC_structure::Halffacet_iterator f;
    CGAL_forall_facets(f, *W) facets.push_back(f);

    std::frexp((double)vertices.size(), &max_depth);
    bounding_box = Bounding_box_3();
    for(auto vi : vertices) bounding_box.extend(vi->point());
    non_efective_splits = 0;
    root = build_kdtree({std::move(vertices), std::move(edges), std::move(facets)}, 0);
  }


private:

int non_efective_splits;

Node_handle build_kdtree(Leaf&& L, int depth) {
  CGAL_precondition( depth >= 0);

  auto num_vertices = L.vertex_list.size();

  if( !can_set_be_divided(depth, num_vertices)) {
    CGAL_NEF_TRACEN("build_kdtree: set cannot be divided");
    nodes.emplace_back(std::move(L));
    return &(nodes.back());
  }

  int coord = depth%3;

  Leaf L1, L2;
  Point_3 point_on_plane;
  classify_vertices(point_on_plane, L.vertex_list, coord, side_of_plane, L1.vertex_list, L2.vertex_list);

  bool edges_not_split = classify_objects(L.edge_list, side_of_plane, L1.edge_list, L2.edge_list);
  if(edges_not_split) {
    CGAL_NEF_TRACEN("build_kdtree: edges splitting plane not found");
    nodes.emplace_back(std::move(L));
    return &(nodes.back());
  }

  bool facets_not_split = classify_objects(L.facet_list, side_of_plane, L1.facet_list, L2.facet_list);
  if(facets_not_split) {
    CGAL_NEF_TRACEN("build_kdtree: facets splitting plane not found");
    nodes.emplace_back(std::move(L));
    return &(nodes.back());
  }

  auto O_size = L.vertex_list.size() + L.edge_list.size() + L.facet_list.size();
  auto O1_size = L1.vertex_list.size() + L1.edge_list.size() + L1.facet_list.size();
  auto O2_size = L2.vertex_list.size() + L2.edge_list.size() + L2.facet_list.size();
  CGAL_NEF_TRACEN("Sizes " << O1_size << ", " << O2_size << ", " << O_size);
  CGAL_assertion( O1_size <= O_size && O2_size <= O_size);
  CGAL_assertion( O1_size + O2_size >= O_size);
  if((O1_size == O_size) || (O2_size == O_size))
    non_efective_splits++;
  else
    non_efective_splits = 0;

  if(non_efective_splits > 2) {
    CGAL_NEF_TRACEN("build_kdtree: non effective splits reached maximum");
    nodes.emplace_back(std::move(L));
    return &(nodes.back());
  }

  Node_handle left_node = build_kdtree(std::move(L1), depth + 1);
  Node_handle right_node = build_kdtree(std::move(L2), depth + 1);
  nodes.emplace_back(left_node, right_node, construct_splitting_plane(point_on_plane, coord, typename Traits::Kernel::Kernel_tag()));
  return &(nodes.back());
}

bool can_set_be_divided(int depth, typename Vertex_list::size_type size) {
  if(depth >= max_depth)
    return false;
  if(size <= 2)
    return false;
  return true;
}

template <typename List>
static bool classify_objects(const List& l,Side_of_plane& sop,
                      List& l1, List& l2) {
  l1.reserve(l.size());
  l2.reserve(l.size());

  for (const auto& o : l) {
    CGAL::Oriented_side side = sop(o);
    // ON_NEGATIVE adds to l1, ON_POSITIVE adds to l2,
    // and ON_ORIENTED_BOUNDARY adds to both.
    if (side != CGAL::ON_POSITIVE_SIDE) l1.push_back(o);
    if (side != CGAL::ON_NEGATIVE_SIDE) l2.push_back(o);
  }

  // Returns true only if every object was on the boundary
  return (l1.size() == l.size() && l2.size() == l.size());
}

template <typename Compare>
struct Smaller {
    Compare compare;
    bool operator()(const Vertex_handle& a, const Vertex_handle& b) const {
        return compare(a->point(), b->point()) == CGAL::SMALLER;
    }
};

template <typename Smaller>
static Point_3 find_median_point(Vertex_list& V, Vertex_iterator& lower, Vertex_iterator& upper) {
    Smaller smaller;

    auto begin = V.begin();
    auto end = V.end();
    upper = std::next(begin, V.size() / 2);
    std::nth_element(begin, upper, end, smaller);
    lower = std::max_element(begin, upper, smaller);

    if (lower == upper) {
      return (*lower)->point();
    }

    if (!smaller(*lower, *upper)) {
      lower = upper;
      return (*lower)->point();
    }

    return CGAL::midpoint((*lower)->point(), (*upper)->point());
}

static Point_3 find_median_point(Vertex_list& V, int coord,
                                 Vertex_iterator& lower, Vertex_iterator& upper) {
    CGAL_assertion(V.size() > 1);
    switch(coord) {
        case 0: return find_median_point<Smaller<typename Kernel::Compare_x_3>>(V, lower, upper);
        case 1: return find_median_point<Smaller<typename Kernel::Compare_y_3>>(V, lower, upper);
        case 2: return find_median_point<Smaller<typename Kernel::Compare_z_3>>(V, lower, upper);
    }

    CGAL_error_msg( "never reached");
    return Point_3();
}

static bool classify_vertices(Point_3& point_on_plane, Vertex_list& V, int coord, Side_of_plane& sop,
                                 Vertex_list& L1, Vertex_list& L2) {
    Vertex_iterator lower, upper;
    point_on_plane = find_median_point(V, coord, lower, upper);

    sop.next(point_on_plane, coord);

    if (lower != upper) {
        L1.reserve(V.size());
        L2.reserve(V.size());
        for (auto it = V.begin(); it != upper; ++it) {
            L1.push_back(*it);
            sop.set_side(*it, CGAL::ON_NEGATIVE_SIDE);
        }
        for (auto it = upper; it != V.end(); ++it) {
            L2.push_back(*it);
            sop.set_side(*it, CGAL::ON_POSITIVE_SIDE);
        }
        return false; // Not all on boundary
    }

    return classify_objects(V, sop, L1, L2);
}

static Plane_3 construct_splitting_plane(const Point_3& pt, int coord, const CGAL::Homogeneous_tag&)
{
  switch(coord) {
  case 0: return Plane_3(pt, Vector_3(1, 0, 0));
  case 1: return Plane_3(pt, Vector_3(0, 1, 0));
  case 2: return Plane_3(pt, Vector_3(0, 0, 1));
  }

  CGAL_error_msg( "never reached");
  return Plane_3();
}

static Plane_3 construct_splitting_plane(const Point_3& pt, int coord, const CGAL::Cartesian_tag&)
{
  switch(coord) {
  case 0: return Plane_3(1, 0, 0, -pt.x());
  case 1: return Plane_3(0, 1, 0, -pt.y());
  case 2: return Plane_3(0, 0, 1, -pt.z());
  }

  CGAL_error_msg( "never reached");
  return Plane_3();
}

};

} // namespace Improved

#endif
