cmake_minimum_required(VERSION 3.29)
project(gfx_testing)
add_compile_options(-Wall -Wpedantic)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

include(cmake/fetch-tinyobjloader.cmake)
set(BOOST_INCLUDE_LIBRARIES algorithm safe_numerics scope)
set(BOOST_ENABLE_CMAKE ON)
include(cmake/fetch-boost.cmake)
include(cmake/fetch-glm.cmake)

find_package(SDL3 REQUIRED)


add_executable(gfx_testing
        main.cpp
        src/obj_loader.cpp
        src/sdl.cpp
        src/util.cpp
        src/scene.cpp
        src/game.cpp
        src/pipelines.cpp
        src/buffer.cpp
)
target_link_libraries(gfx_testing
        SDL3::SDL3
        Boost::algorithm
        Boost::safe_numerics
        Boost::scope
        glm::glm
)
target_include_directories(gfx_testing SYSTEM PUBLIC ${FETCHCONTENT_BASE_DIR}/tinyobjloader-src)
target_include_directories(gfx_testing PRIVATE src/)
