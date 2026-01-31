# main renderer executable
find_package(clipp CONFIG REQUIRED)
find_package(toml11 CONFIG REQUIRED)

add_executable(renderer main.cpp
        cli.hpp)
target_link_libraries(renderer PRIVATE
        clipp::clipp
        toml11::toml11
        gfx_testing_lib
)
target_include_directories(renderer SYSTEM PRIVATE src/)