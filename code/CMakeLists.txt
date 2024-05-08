cmake_minimum_required(VERSION 3.27)

set(CMAKE_C_STANDARD 23)
SET(CMAKE_CXX_FLAGS "-Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")

include(Dependencies.cmake)
ConstraintBasedSimulator_setup_dependencies()

project(simulator C)

# TODO make lib with source files

add_executable(simulator main.c
    simulator.c
    symdiff.c
    matrixn.c
    math.c
    constraint_type.c
    cases.c)

target_compile_options(simulator PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_link_libraries(simulator raylib)

add_executable(test
    simulator.c
    symdiff.c
    matrixn.c
    math.c
    constraint_type.c
    test_symdiff_node.c
    test_symdiff_matrix.c
    test_matrixn.c)

target_compile_options(test PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_link_libraries(test raylib)
target_link_libraries(test criterion)
