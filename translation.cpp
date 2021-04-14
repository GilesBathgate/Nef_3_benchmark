#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/IO/Nef_polyhedron_iostream_3.h>
#include <CGAL/OFF_to_nef_3.h>
#include <CGAL/Timer.h>

#include <iostream>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef CGAL::Polyhedron_3<K> Polyhedron;
typedef CGAL::Nef_polyhedron_3<K> Nef_polyhedron;
typedef CGAL::Aff_transformation_3<K> Transformation;

template <class Poly>
typename Poly::Halfedge_handle make_cube_3( Poly& P) {
    // appends a cube of size [0,1]^3 to the polyhedron P.
    CGAL_precondition( P.is_valid());
    typedef typename Poly::Point_3         Point;
    typedef typename Poly::Halfedge_handle Halfedge_handle;
    Halfedge_handle h = P.make_tetrahedron( Point( 1, 0, 0),
                                            Point( 0, 0, 1),
                                            Point( 0, 0, 0),
                                            Point( 0, 1, 0));
    Halfedge_handle g = h->next()->opposite()->next();             // Fig. (a)
    P.split_edge( h->next());
    P.split_edge( g->next());
    P.split_edge( g);                                              // Fig. (b)
    h->next()->vertex()->point()     = Point( 1, 0, 1);
    g->next()->vertex()->point()     = Point( 0, 1, 1);
    g->opposite()->vertex()->point() = Point( 1, 1, 0);            // Fig. (c)
    Halfedge_handle f = P.split_facet( g->next(),
                                       g->next()->next()->next()); // Fig. (d)
    Halfedge_handle e = P.split_edge( f);
    e->vertex()->point() = Point( 1, 1, 1);                        // Fig. (e)
    P.split_facet( e, f->next()->next());                          // Fig. (f)
    CGAL_postcondition( P.is_valid());
    return h;
}

int main(int argc, char* argv[])
{

  CGAL::Timer t;
  t.start();

  Nef_polyhedron result;
  for(int i=0; i<200; ++i)
  {
    Polyhedron p;
    make_cube_3(p);
    Nef_polyhedron np(p);
    double x=10,y=0,z=0;
    Transformation translate(1,0,0,x,
                             0,1,0,y,
                             0,0,1,z,
                                   1);
    /* moving the result is deliberately slower we could
       of course get the same effect by moving p, or np */
    result.transform(translate);
    result = result.join(np);
  }
  std::cout << result.number_of_vertices() << " vertices  " << std::endl;
  std::cout << "Total  : " << t.time() << " sec"  << std::endl;
  return 0;
}
