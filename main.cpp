#include <filesystem>
#include <SDL3/SDL.h>
#include <obj_loader.hpp>
#include <sdl.hpp>
#include <util.hpp>

void handleEvent(SDL_Event const &event) {
    // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
}

int main() {
    gfx_testing::sdl::SdlContext context;
    gfx_testing::model::loadObjFile(gfx_testing::util::getProjectRoot() / "content/basic-shapes.obj");
    gfx_testing::sdl::runEventLoop(handleEvent);
    return 0;
}
