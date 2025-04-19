#include <filesystem>
#include <SDL3/SDL.h>
#include <sdl.hpp>
#include <util.hpp>
#include <scene.hpp>
#include <game.hpp>

void handleEvent(gfx_testing::game::GameContext const &gameContext, SDL_Event const &event) {
    // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
}

void handleUpdate(gfx_testing::game::GameContext const &, gfx_testing::scene::Scene &scene) {
    scene.update();
    scene.draw();
}

int main() {
    const gfx_testing::sdl::SdlContext sdlContext;
    gfx_testing::game::GameContext gameContext(sdlContext);
    const auto projectRoot = gfx_testing::util::getProjectRoot();

    gfx_testing::scene::Scene scene(gameContext, projectRoot);

    auto updateFunction = [&scene](auto const &gameCtx) { handleUpdate(gameCtx, scene); };
    SDL_Log("Begin main loop");
    gameContext.runMainLoop(handleEvent, updateFunction);
    return 0;
}
