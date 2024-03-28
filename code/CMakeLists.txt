cmake_minimum_required(VERSION 3.27)

set(CMAKE_C_STANDARD 23)

include(Dependencies.cmake)
ConstraintBasedSimulator_setup_dependencies()

project(simulator C)

add_executable(simulator main.c
    simulator.c
    simulator.h
    symdiff.c
    symdiff.h)

target_compile_options(simulator PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_link_libraries(simulator raylib)

add_executable(test test.c
    simulator.c
    simulator.h
    symdiff.c
    symdiff.h)

target_compile_options(test PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_link_libraries(test raylib)
