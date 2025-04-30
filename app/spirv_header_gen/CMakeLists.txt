# main renderer executable
add_executable(spirv_header_gen main.cpp
        parse.cpp
        write.cpp
        write.hpp)
target_link_libraries(spirv_header_gen PRIVATE
        gfx_testing_lib
        clipp::clipp
        nlohmann_json::nlohmann_json
)
target_include_directories(spirv_header_gen SYSTEM PRIVATE .)