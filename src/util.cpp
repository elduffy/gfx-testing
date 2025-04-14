#include <util.hpp>

namespace gfx_testing::util {
    std::filesystem::path getProjectRoot() {
        std::filesystem::path path = std::filesystem::current_path();
        while (!is_empty(path)) {
            if (path.filename() == "gfx-testing") {
                return path;
            }
            path = path.parent_path();
        }
        throw std::runtime_error("Could not find project root directory.");
    }
}
