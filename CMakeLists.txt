# Created by the script cgal_create_cmake_script
# This is the CMake script for compiling a CGAL application.

cmake_minimum_required(VERSION 3.1...3.15)
project(Nef_3_benchmark)

set(CMAKE_BUILD_TYPE Release)
add_definitions(-DNDEBUG)

find_package(CGAL REQUIRED)

find_package(benchmark REQUIRED)

add_executable(k3_tree_benchmark k3_tree_benchmark.cpp)
target_link_libraries(k3_tree_benchmark PRIVATE CGAL::CGAL benchmark::benchmark)
