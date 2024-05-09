cmake_minimum_required(VERSION 3.27)

set(CMAKE_C_STANDARD 23)
SET(CMAKE_CXX_FLAGS "-Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")

include(Dependencies.cmake)
ConstraintBasedSimulator_setup_dependencies()

project(simulator C)

SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror")

add_library(simulator_lib
    simulator.c
    symdiff.c
    matrixn.c
    math.c
    constraint_type.c
    cases.c)

target_link_libraries(simulator_lib raylib)

add_executable(simulator main.c)

target_link_libraries(simulator simulator_lib)

add_executable(tests
    test_symdiff_node.c
    test_symdiff_matrix.c
    test_matrixn.c
)

target_link_libraries(tests criterion simulator_lib)
