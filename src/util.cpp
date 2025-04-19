#include <algorithm>
#include <format>
#include <sdl.hpp>
#include <util.hpp>
#include <SDL3/SDL_log.h>
#include <boost/scope/scope_exit.hpp>

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
