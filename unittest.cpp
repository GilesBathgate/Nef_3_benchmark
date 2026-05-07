#define CGAL_NEF_NO_INDEXED_ITEMS 1
#include "Default_items.h"
#include "vertex_cycle_to_nef_3.h"

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <vector>

typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef CGAL::Nef_polyhedron_3<K> Nef_polyhedron;
typedef typename K::Point_3 Point_3;

int main(int argc, char* argv[])
{
  std::vector<Point_3> points { {0,0,0}, {0,1,0} };
  Nef_polyhedron nefA(points.begin(), points.end());

  Nef_polyhedron nefB;

  auto result = nefA.intersection(nefB);

  return 0;
}
