// Copyright (c) 1997-2002  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Michael Seel       <seel@mpi-sb.mpg.de>
//                 Peter Hachenberger <hachenberger@mpi-sb.mpg.de>
#ifndef CGAL_SNC_INTERSECTION_H
#define CGAL_SNC_INTERSECTION_H

#include <CGAL/license/Nef_3.h>


#include <CGAL/basic.h>
#include <CGAL/Circulator_project.h>

#undef CGAL_NEF_DEBUG
#define CGAL_NEF_DEBUG 37
#include <CGAL/Nef_2/debug.h>

namespace CGAL {

template < class Node, class Object>
struct Project_shalfedge_point {
  typedef Node         argument_type;
  typedef Object       result_type;

  const Object& operator()( const Node& x) const   {
    return x.source()->source()->point();
    /* a Point_3& reference must be returned by D.point() */
  }
};

template<typename SNC_structure_>
class SNC_intersection {

  typedef SNC_structure_                     SNC_structure;
  typedef SNC_intersection<SNC_structure>    Self;

  typedef typename SNC_structure::SHalfedge               SHalfedge;
  typedef typename SNC_structure::Halfedge_handle         Halfedge_handle;
  typedef typename SNC_structure::Halfedge_const_handle   Halfedge_const_handle;
  typedef typename SNC_structure::Halffacet_const_handle
                                  Halffacet_const_handle;
  typedef typename SNC_structure::SHalfedge_const_handle  SHalfedge_const_handle;
  typedef typename SNC_structure::SHalfloop_const_handle  SHalfloop_const_handle;
  typedef typename SNC_structure::SHalfedge_around_facet_const_circulator
                                  SHalfedge_around_facet_const_circulator;
  typedef typename SNC_structure::Halffacet_cycle_const_iterator
                                  Halffacet_cycle_const_iterator;

  typedef typename SNC_structure::Point_3        Point_3;
  typedef typename SNC_structure::Vector_3       Vector_3;
  typedef typename SNC_structure::Segment_3      Segment_3;
  typedef typename SNC_structure::Line_3         Line_3;
  typedef typename SNC_structure::Ray_3          Ray_3;
  typedef typename SNC_structure::Plane_3        Plane_3;
  typedef typename SNC_structure::Kernel         Kernel;

 public:

  static bool does_contain_internally(const Point_3& s,
                                      const Point_3& t,
                                      const Point_3& p) {
    return are_strictly_ordered_along_line (s, p, t);
  }

  static bool does_contain_internally(Halffacet_const_handle f,
                                      const Point_3& p) {
    if (!f->plane().has_on(p))
      return false;

    return point_in_facet_interior(p, f);
  }

  static bool does_intersect_internally(Halfedge_const_handle e1,
                                        Halfedge_const_handle e2,
                                        Point_3& p) {

    const Point_3& e1_src = e1->source()->point();
    const Point_3& e1_tgt = e1->twin()->source()->point();
    const Point_3& e2_src = e2->source()->point();
    const Point_3& e2_tgt = e2->twin()->source()->point();

    Segment_3 s1(e1_src, e1_tgt);
    if (s1.has_on(e2_src) || s1.has_on(e2_tgt))
      return false;

    Segment_3 s2(e2_src, e2_tgt);
    if (s2.has_on(e1_src) || s2.has_on(e1_tgt))
      return false;

    return does_intersect(s1, s2, p);
  }

  static bool does_intersect_internally(Halfedge_const_handle e1,
                                        Halffacet_const_handle f2,
                                        Point_3& p) {

    const Point_3& src = e1->source()->point();
    const Point_3& tgt = e1->twin()->source()->point();
    const Plane_3& h = f2->plane();

    if (h.has_on(src) || h.has_on(tgt))
      return false;

    Segment_3 s(src, tgt);
    if (!does_intersect(h, s, p))
      return false;

    return point_in_facet_interior(p, f2);
  }

  static bool does_intersect_internally(const Segment_3& s1,
                                        Halfedge_const_handle e2,
                                        Point_3& p) {

    const Point_3& e2_src = e2->source()->point();
    const Point_3& e2_tgt = e2->twin()->source()->point();

    if (s1.has_on(e2_src) || s1.has_on(e2_tgt))
      return false;

    Segment_3 s2(e2_src, e2_tgt);
    if (s2.has_on(s1.source()) || s2.has_on(s1.target()))
      return false;

    return does_intersect(s1, s2, p);
  }

  static bool does_intersect_internally(const Segment_3& s1,
                                        Halffacet_const_handle f2,
                                        Point_3& p) {

    const Plane_3& h = f2->plane();

    if (h.has_on(s1.source()) || h.has_on(s1.target()))
      return false;

    if (!does_intersect(h, s1, p))
      return false;

    return point_in_facet_interior(p, f2);
  }

  static bool does_intersect_internally(const Ray_3& r1,
                                        Halfedge_const_handle e2,
                                        Point_3& p) {

    const Point_3& e2_src = e2->source()->point();
    const Point_3& e2_tgt = e2->twin()->source()->point();

    if (r1.has_on(e2_src) || r1.has_on(e2_tgt))
      return false;

    Segment_3 s2(e2_src, e2_tgt);
    if (s2.has_on(r1.source()))
      return false;

    return does_intersect(r1, s2, p);
  }

  static bool does_intersect_internally(const Ray_3& r1,
                                        Halffacet_const_handle f2,
                                        Point_3& p) {

    const Plane_3& h = f2->plane();

    if (h.has_on(r1.source()))
      return false;

    if (!does_intersect(h, r1, p))
      return false;

    return point_in_facet_interior(p, f2);
  }

 private:

  template <typename T1, typename T2>
  static bool does_intersect(const T1& o1, const T2& o2, Point_3& p)
  {
    const auto o = intersection(o1, o2);
    if (!o)
      return false;

    const Point_3* ip = std::get_if<Point_3>(&*o);
    if (!ip)
        return false;

    p = *ip;
    return true;
  }

  static bool point_in_facet_interior(const Point_3& p,
                                      Halffacet_const_handle f) {
    return (locate_point_in_halffacet( p, f) == CGAL::ON_BOUNDED_SIDE);
  }

  static Bounded_side locate_point_in_halffacet(const Point_3& p,
                                                Halffacet_const_handle f) {
    CGAL_NEF_TRACEN("locate point in halffacet " << p << ", " << f->plane());
    typedef Project_shalfedge_point
      < SHalfedge, const Point_3> Project;
    typedef Circulator_project
      < SHalfedge_around_facet_const_circulator, Project,
      const Point_3&, const Point_3*> Circulator;
    typedef Container_from_circulator<Circulator> Container;

    Plane_3 h(f->plane());
    //CGAL_assertion(h.has_on(p));
    //CGAL_assertion(!h.is_degenerate());

    typename Kernel::Non_zero_coordinate_index_3 non_zero_coordinate_index_3;
    int coord = non_zero_coordinate_index_3(h.orthogonal_vector());

    Halffacet_cycle_const_iterator fc = f->facet_cycles_begin();
    Bounded_side outer_bound_pos(CGAL::ON_BOUNDARY);
    if (fc.is_shalfedge() ) {
      SHalfedge_const_handle se(fc);
      SHalfedge_around_facet_const_circulator hfc(se);
      Circulator c(hfc);
      Container ct(c);
      //GAL_assertion( !is_empty_range(ct.begin(), ct.end()));
      outer_bound_pos = bounded_side_3(ct.begin(), ct.end(), p, coord);
    }
    else
      //CGAL_error_msg( "is facet first cycle a SHalfloop?");
      assert(false);
    if( outer_bound_pos != CGAL::ON_BOUNDED_SIDE )
      return outer_bound_pos;
    /* The point p is not in the relative interior of the outer face cycle
       so it is not necessary to know the position of p with respect to the
       inner face cycles */
    Halffacet_cycle_const_iterator fe = f->facet_cycles_end();
    ++fc;
    if( fc == fe )
      return outer_bound_pos;
    Bounded_side inner_bound_pos(CGAL::ON_BOUNDARY);
    CGAL_For_all(fc, fe) {
      if (fc.is_shalfloop() ) {
        SHalfloop_const_handle l(fc);
        if(l->incident_sface()->center_vertex()->point() == p )
          inner_bound_pos = CGAL::ON_BOUNDARY;
        else
          inner_bound_pos = CGAL::ON_UNBOUNDED_SIDE;
      }
      else if (fc.is_shalfedge() ) {
        SHalfedge_const_handle se(fc);
        SHalfedge_around_facet_const_circulator hfc(se);
        Circulator c(hfc);
        Container ct(c);
        //CGAL_assertion( !is_empty_range(ct.begin(), ct.end()));
        inner_bound_pos = bounded_side_3( ct.begin(), ct.end(),
                                          p, coord);
      }
      else
        //CGAL_error_msg( "Damn wrong handle.");
        assert(false);
      if( inner_bound_pos != CGAL::ON_UNBOUNDED_SIDE )
        return opposite(inner_bound_pos);
      /* At this point the point p belongs to relative interior of the facet's
         outer cycle, and its position is completely known when it belongs
         to the clousure of any inner cycle */
    }
    return CGAL::ON_BOUNDED_SIDE;
  }

}; // SNC_intersection

} //namespace CGAL

#endif //CGAL_SNC_INTERSECTION_H
