find_package(clipp CONFIG REQUIRED)
find_package(inja CONFIG REQUIRED)

add_executable(spirv_header_gen
        main.cpp
        write.cpp
)
target_link_libraries(spirv_header_gen PRIVATE
        clipp::clipp
        nlohmann_json::nlohmann_json
        pantor::inja
        absl::absl_check
        absl::log
        absl::strings
)
target_include_directories(spirv_header_gen SYSTEM PRIVATE .)