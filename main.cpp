#include <filesystem>
#include <iostream>
#include <SDL3/SDL.h>
#include <obj_loader.hpp>

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
    std::cout << "Hello, World!" << std::endl;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }
	SDL_Log("SDL initialized.");

    SDL_Window* window = SDL_CreateWindow("gfx-testing", 768, 512, SDL_WINDOW_RESIZABLE);
    if (window == nullptr)
    {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return -1;
    }

    model::loadObjFile( getProjectRoot() / "content/basic-shapes.obj");

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

	SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
