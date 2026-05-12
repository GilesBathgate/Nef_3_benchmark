cmake_minimum_required(VERSION 3.15)
project(Nef_3_test)

find_package(CGAL REQUIRED)
create_single_source_cgal_program(test_id_support_direct.cpp)
