#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/IO/Nef_polyhedron_iostream_3.h>
#include <CGAL/OFF_to_nef_3.h>
#include <CGAL/Timer.h>

#include <fstream>
#include <iostream>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef CGAL::Surface_mesh<K::Point_3> Surface_mesh;
typedef CGAL::Nef_polyhedron_3<K> Nef_polyhedron;


int main(int argc, char* argv[])
{
  std::cout << argv[0] << std::endl;

  Surface_mesh A, B;

  {
    std::ifstream in ((argc > 1) ? argv[1] : "data/cube.off");
    in >> A;
  }

  {
    std::ifstream in((argc > 2) ? argv[2] : "data/cylinders.off");
    in >> B;
  }

  CGAL::Timer tglobal, t;
  double t_reading, t_boolean;
  tglobal.start();
  {
    t.start();
    Nef_polyhedron nefA(A), nefB(B);
    t_reading = t.time();
    t.reset();

    Nef_polyhedron nefC = nefA.difference(nefB);
    t_boolean = t.time();

    std::cout << nefC.number_of_vertices() << " vertices  " << std::endl;

  }

  std::cout << "Reading: " << t_reading << " sec"  << std::endl;
  std::cout << "Boolean: " << t_boolean << " sec"  << std::endl;
  std::cout << "Total  : " << tglobal.time() << " sec"  << std::endl;
  return 0;
}
