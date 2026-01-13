# Requires SHADER_HPP_INCLUDE
# Defines build_shaders command

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