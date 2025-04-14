#include <filesystem>
#include <iostream>
#include <SDL3/SDL.h>
#include <obj_loader.hpp>
#include <sdl.hpp>

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

int main() {
    gfx_testing::sdl::SdlContext context;
    gfx_testing::model::loadObjFile(getProjectRoot() / "content/basic-shapes.obj");

    bool quit = false;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT: {
                    quit = true;
                    SDL_Log("Quitting.");
                    break;
                }
                default: {
                    // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
                    break;
                }
            }
            if (quit) {
                break;
            }
        }
    }

    return 0;
}
