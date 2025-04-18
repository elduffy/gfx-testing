#include <filesystem>
#include <SDL3/SDL.h>
#include <sdl.hpp>
#include <util.hpp>
#include <scene.hpp>

void handleEvent(SDL_Event const &event) {
    // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
}

int main() {
    gfx_testing::sdl::SdlContext context;
    const auto projectRoot = gfx_testing::util::getProjectRoot();

    gfx_testing::scene::Scene scene(context, projectRoot);
    scene.Update(context);

    gfx_testing::sdl::runEventLoop(handleEvent);
    return 0;
}
