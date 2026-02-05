cmake_minimum_required(VERSION 3.28)
project(gfx_testing)
add_compile_options(-Wall -Wpedantic)
set(CMAKE_CXX_STANDARD 20)

set(SDL_STATIC ON)
find_package(SDL3 CONFIG REQUIRED SDL3 SDL3-static)
find_package(SDL3_image CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED algorithm safe_numerics scope)
find_package(glm CONFIG REQUIRED glm)
find_package(absl CONFIG REQUIRED base absl_check log)
find_package(Catch2 CONFIG REQUIRED Catch2WithMain)
find_package(EnTT CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED fastgltf)
find_package(nlohmann_json CONFIG REQUIRED nlohmann_json)
find_package(imgui CONFIG REQUIRED)

add_subdirectory(app/spirv_header_gen)

# Shader compilation

file(GLOB_RECURSE SHADER_SOURCES content/shaders/src/*.hlsl)
file(GLOB_RECURSE SHADER_INCLUDES content/shaders/include/*.hlsl)

set(SHADER_HPP_INCLUDE ${PROJECT_BINARY_DIR}/shaders/compiled/headers/)
include(cmake/build-shaders.cmake)

# gfx_testing_lib main library
add_library(gfx_testing_lib
        src/game.cpp
        src/debug/imgui_context.cpp
        src/debug/imgui_ecs_view.cpp
        src/ecs/ecs.cpp
        src/io/input_manager.cpp
        src/io/gltf_loader.cpp
        src/io/obj_loader.cpp
        src/io/resource_loader.cpp
        src/io/texture_loader.cpp
        src/pipeline/compute/pipeline.cpp
        src/pipeline/gfx/pipeline.cpp
        src/pipeline/pipelines.cpp
        src/render/camera.cpp
        src/render/debug_axes.cpp
        src/render/debug_normals.cpp
        src/render/draw_context.cpp
        src/render/point_light.cpp
        src/render/render_object.cpp
        src/render/samplers.cpp
        src/render/scene.cpp
        src/render/scene_objects.cpp
        src/render/sky_box.cpp
        src/sdl.cpp
        src/sdl_factories.cpp
        src/shader/object.cpp
        src/util/cube_map.cpp
        src/util/fps_capper.cpp
        src/util/mesh.cpp
        src/util/scoped_timer.cpp
        src/util/stopwatch.cpp
        src/util/util.cpp
        src/ecs/entity.hpp
)
target_link_libraries(gfx_testing_lib PUBLIC
        imgui::imgui
        SDL3::SDL3
        SDL3_image::SDL3_image
        absl::base
        absl::absl_check
        absl::log
        Boost::algorithm
        Boost::safe_numerics
        Boost::scope
        glm::glm
        fastgltf::fastgltf
        EnTT::EnTT
)
add_dependencies(gfx_testing_lib build_shaders)
target_include_directories(gfx_testing_lib SYSTEM PUBLIC src/)
target_include_directories(gfx_testing_lib SYSTEM PUBLIC ${SHADER_HPP_INCLUDE})

add_subdirectory(app/renderer)

# tests
add_executable(tests
        tests/io/gltf_loader_tests.cpp
        tests/io/obj_loader_tests.cpp
        tests/shader/mesh_tests.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain gfx_testing_lib)
target_include_directories(tests SYSTEM PRIVATE src/)