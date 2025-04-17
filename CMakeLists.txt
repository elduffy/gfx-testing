cmake_minimum_required(VERSION 3.29)
project(gfx_testing)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

include(cmake/fetch-tinyobjloader.cmake)
set(BOOST_INCLUDE_LIBRARIES algorithm)
set(BOOST_ENABLE_CMAKE ON)
include(cmake/fetch-boost.cmake)

find_package(SDL3 REQUIRED)


add_executable(gfx_testing
        main.cpp
        src/obj_loader.cpp
        src/sdl.cpp
        src/util.cpp
)
target_link_libraries(gfx_testing
        SDL3::SDL3
        Boost::algorithm
)
target_include_directories(gfx_testing SYSTEM PUBLIC ${FETCHCONTENT_BASE_DIR}/tinyobjloader-src)
target_include_directories(gfx_testing PRIVATE src/)
