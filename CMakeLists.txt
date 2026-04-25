# Created by the script cgal_create_cmake_script
# This is the CMake script for compiling a CGAL application.

cmake_minimum_required(VERSION 3.1...3.15)
project(Nef_3_benchmark)

find_package(CGAL REQUIRED OPTIONAL_COMPONENTS Qt6)

if(CGAL_Qt6_FOUND)
  add_definitions(-DCGAL_USE_BASIC_VIEWER -DQT_NO_KEYWORDS)
endif()

# create a target per cppfile
file(
  GLOB cppfiles
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
foreach(cppfile ${cppfiles})
  if(NOT ${cppfile} STREQUAL "median_benchmark.cpp")
    create_single_source_cgal_program("${cppfile}")
  endif()
endforeach()

find_package(benchmark REQUIRED)
add_executable(median_benchmark median_benchmark.cpp)
target_link_libraries(median_benchmark PRIVATE CGAL::CGAL benchmark::benchmark)

if(CGAL_Qt6_FOUND)
#  target_link_libraries(draw_nef_3 PUBLIC CGAL::CGAL_Qt6)
endif()
