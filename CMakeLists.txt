cmake_minimum_required(VERSION 3.29)
project(gfx_testing)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL3 REQUIRED)

add_executable(gfx_testing main.cpp)

target_link_libraries(gfx_testing
        SDL3::SDL3
)
