add_executable(spirv_header_gen
        main.cpp
        write.cpp
)
target_link_libraries(spirv_header_gen PRIVATE
        clipp::clipp
        nlohmann_json::nlohmann_json
        pantor::inja
        Boost::algorithm
        absl::absl_check
        absl::log
)
target_include_directories(spirv_header_gen SYSTEM PRIVATE .)