# Created by the script cgal_create_cmake_script
# This is the CMake script for compiling a CGAL application.

cmake_minimum_required(VERSION 3.1...3.15)
project(Nef_3_benchmark)

find_package(CGAL REQUIRED OPTIONAL_COMPONENTS Qt5)
find_package(benchmark REQUIRED)


create_single_source_cgal_program(benchmarks.cpp)
target_link_libraries(benchmarks PUBLIC benchmark::benchmark)

