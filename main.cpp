#include <filesystem>
#include <SDL3/SDL.h>
#include <sdl.hpp>
#include <util.hpp>
#include <scene.hpp>
#include <game.hpp>

void handleEvent(gfx_testing::scene::Scene &scene, SDL_Event const &event) {
    switch (event.type) {
        case SDL_EVENT_WINDOW_RESIZED: {
            scene.onResize({
                    boost::safe_numerics::checked::cast<uint32_t>(event.window.data1),
                    boost::safe_numerics::checked::cast<uint32_t>(event.window.data2),
            });
            break;
        }
        default: {
            // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
            break;
        }
    }
}

void handleUpdate(gfx_testing::scene::Scene &scene) {
    scene.update();
    scene.draw();
}

int main() {
    const gfx_testing::sdl::SdlContext sdlContext;
    const auto projectRoot = gfx_testing::util::getProjectRoot();
    gfx_testing::game::GameContext gameContext(sdlContext, projectRoot);

    gfx_testing::scene::Scene scene(gameContext, projectRoot);

    auto eventFunction = [&scene](auto const &event) { handleEvent(scene, event); };
    auto updateFunction = [&scene] { handleUpdate(scene); };
    SDL_Log("Begin main loop");
    gameContext.runMainLoop(eventFunction, updateFunction);
    return 0;
}
