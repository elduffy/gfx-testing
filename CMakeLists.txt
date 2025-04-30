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
include(cmake/fetch-catch2.cmake)
include(cmake/fetch-imgui.cmake)
include(cmake/fetch-clipp.cmake)
include(cmake/fetch-nlohmann_json.cmake)

find_package(SDL3 REQUIRED)

add_subdirectory(${SDL_IMAGE_DIR} ${SDL_IMAGE_DIR}/build EXCLUDE_FROM_ALL)

add_custom_target(build_shaders
        COMMAND bash content/shaders/build-shaders.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..
)

# imgui build
add_library(imgui
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlgpu3.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

# gfx_testing_lib main library
add_library(gfx_testing_lib
        src/obj_loader.cpp
        src/sdl.cpp
        src/util.cpp
        src/scene.cpp
        src/game.cpp
        src/pipelines.cpp
        src/render_object.cpp
        src/stopwatch.cpp
        src/camera.cpp
        src/point_light.cpp
        src/resource_loader.cpp
        src/samplers.cpp
        src/texture.cpp
        src/imgui_context.cpp
        src/debug_axes.cpp
        src/spirv_header.cpp
)
target_link_libraries(gfx_testing_lib PUBLIC
        imgui
        SDL3::SDL3
        SDL3_image::SDL3_image
        Boost::algorithm
        Boost::safe_numerics
        Boost::scope
        glm::glm
)
add_dependencies(gfx_testing_lib build_shaders)
target_include_directories(gfx_testing_lib SYSTEM PUBLIC ${FETCHCONTENT_BASE_DIR}/tinyobjloader-src)
target_include_directories(gfx_testing_lib SYSTEM PUBLIC src/)

add_subdirectory(app/renderer)
add_subdirectory(app/spirv_header_gen)

# tests
add_executable(tests
        tests/obj_loader_tests.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain gfx_testing_lib)
target_include_directories(tests SYSTEM PRIVATE src/)