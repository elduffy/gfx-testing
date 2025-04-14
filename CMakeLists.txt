cmake_minimum_required(VERSION 3.29)
project(gfx_testing)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

include(cmake/fetch-tinyobjloader.cmake)

find_package(SDL3 REQUIRED)


add_executable(gfx_testing
        main.cpp
        src/obj_loader.cpp
        src/sdl.cpp
)
target_link_libraries(gfx_testing
        SDL3::SDL3
)
target_include_directories(gfx_testing SYSTEM PUBLIC ${FETCHCONTENT_BASE_DIR}/tinyobjloader-src)
target_include_directories(gfx_testing PRIVATE src/)
