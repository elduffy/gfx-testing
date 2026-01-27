# main renderer executable
find_package(clipp CONFIG REQUIRED)
add_executable(renderer main.cpp)
target_link_libraries(renderer PRIVATE
        clipp::clipp
        gfx_testing_lib
)
target_include_directories(renderer SYSTEM PRIVATE src/)