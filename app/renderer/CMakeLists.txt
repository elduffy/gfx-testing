# main renderer executable
add_executable(renderer main.cpp)
target_link_libraries(renderer PRIVATE
        gfx_testing_lib
)
target_include_directories(renderer SYSTEM PRIVATE src/)