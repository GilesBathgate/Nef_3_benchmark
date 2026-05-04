cmake_minimum_required(VERSION 3.15)
project(Nef_3_benchmark)

find_package(CGAL REQUIRED)
find_package(benchmark REQUIRED)
add_executable(intersection_benchmark intersection_benchmark.cpp)
target_link_libraries(intersection_benchmark PRIVATE CGAL::CGAL benchmark::benchmark)
