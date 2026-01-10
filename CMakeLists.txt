cmake_minimum_required(VERSION 3.28)
project(gfx_testing)
add_compile_options(-Wall -Wpedantic)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

include(cmake/fetch-tinyobjloader.cmake)
include(cmake/fetch-catch2.cmake)
include(cmake/fetch-imgui.cmake)
include(cmake/fetch-clipp.cmake)
include(cmake/fetch-nlohmann_json.cmake)
set(INJA_USE_EMBEDDED_JSON OFF CACHE BOOL "Use fetched nlohmann_json")
include(cmake/fetch-inja.cmake)
include(cmake/fetch-fastgltf.cmake)

find_package(SDL3 REQUIRED SDL3)
find_package(Boost CONFIG REQUIRED algorithm safe_numerics scope)
find_package(glm CONFIG REQUIRED glm)
find_package(absl CONFIG REQUIRED base absl_check log)
find_package(EnTT CONFIG REQUIRED)

add_subdirectory(${SDL_IMAGE_DIR} ${SDL_IMAGE_DIR}/build EXCLUDE_FROM_ALL)
add_subdirectory(app/spirv_header_gen)

# Shader compilation

file(GLOB_RECURSE SHADER_SOURCES content/shaders/src/*.hlsl)
file(GLOB_RECURSE SHADER_INCLUDES content/shaders/include/*.hlsl)

set(SHADER_HPP_INCLUDE ${PROJECT_BINARY_DIR}/shaders/compiled/headers/)

set(SPIRV_OPT_OPTS "")
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND SPIRV_OPT_OPTS "-O" "--strip-debug" "--strip-nonsemantic")
elseif (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    list(APPEND SPIRV_OPT_OPTS "-Os" "--strip-debug" "--strip-nonsemantic")
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    list(APPEND SPIRV_OPT_OPTS "-O")
endif ()

foreach (SHADER_SOURCE ${SHADER_SOURCES})
    get_filename_component(SHADER_SOURCE_FILENAME ${SHADER_SOURCE} NAME)
    string(REPLACE .hlsl "" OUTFILE_PREFIX ${SHADER_SOURCE_FILENAME})
    set(SPV_OUT ${PROJECT_BINARY_DIR}/shaders/compiled/spirv/${OUTFILE_PREFIX}.spv)
    set(JSON_OUT ${PROJECT_BINARY_DIR}/shaders/compiled/json/${OUTFILE_PREFIX}.json)
    set(HPP_OUT ${SHADER_HPP_INCLUDE}/${OUTFILE_PREFIX}.hpp)
    # HLSL -> SPV and SPV -> JSON metadata
    add_custom_command(
            OUTPUT ${SPV_OUT} ${JSON_OUT}
            DEPENDS ${SHADER_SOURCE} ${SHADER_INCLUDES}
            COMMAND shadercross ${SHADER_SOURCE} -o ${SPV_OUT} -I ${CMAKE_SOURCE_DIR}/content/shaders/include
            # Generate JSON before stripping symbols
            # TODO: use unstripped to generate the symbols in the headers, but stripped for the string values.
            COMMAND spirv-cross ${SPV_OUT} --reflect --output ${JSON_OUT}
            COMMAND spirv-opt ${SPIRV_OPT_OPTS} ${SPV_OUT} -o ${SPV_OUT}
    )
    list(APPEND SPV_OUTFILES ${SPV_OUT})
    list(APPEND JSON_OUTFILES ${JSON_OUT})
    # JSON -> CPP headers
    add_custom_command(
            OUTPUT ${HPP_OUT}
            DEPENDS spirv_header_gen ${JSON_OUT} app/spirv_header_gen/template.hpp.inja
            COMMAND spirv_header_gen ${JSON_OUT} -o ${HPP_OUT}
    )
    list(APPEND HPP_OUTFILES ${HPP_OUT})
endforeach ()
add_custom_target(build_shaders ALL DEPENDS ${SPV_OUTFILES} ${JSON_OUTFILES} ${HPP_OUTFILES})

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
        src/game.cpp
        src/imgui_context.cpp
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
)
target_link_libraries(gfx_testing_lib PUBLIC
        imgui
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
target_include_directories(gfx_testing_lib SYSTEM PUBLIC ${FETCHCONTENT_BASE_DIR}/tinyobjloader-src)
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