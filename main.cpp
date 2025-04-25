#include <filesystem>
#include <SDL3/SDL.h>
#include <sdl.hpp>
#include <util.hpp>
#include <scene.hpp>
#include <game.hpp>

void handleEvent(gfx_testing::game::GameContext &gameContext, gfx_testing::scene::Scene &scene,
                 SDL_Event const &event) {
    switch (event.type) {
        case SDL_EVENT_WINDOW_RESIZED: {
            scene.onResize({
                    boost::safe_numerics::checked::cast<uint32_t>(event.window.data1),
                    boost::safe_numerics::checked::cast<uint32_t>(event.window.data2),
            });
            break;
        }
        case SDL_EVENT_KEY_UP: {
            if (!event.key.down && !event.key.repeat && event.key.key == SDLK_RETURN) {
                gameContext.mStopwatch.toggle();
            }
            break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
            if ((event.motion.state & SDL_BUTTON_MMASK) != 0) {
                constexpr auto RADS_PER_VIEWPORT_DIMENSIONS = 4.f;
                auto const extent = scene.getViewportExtent().asVec2() / RADS_PER_VIEWPORT_DIMENSIONS;
                glm::vec2 const &radians = {-event.motion.yrel / extent.y, -event.motion.xrel / extent.x};
                scene.getCamera().pivot(radians);
            }
            break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
            constexpr auto UNITS_PER_MOUSE_CLICK = 0.5f;
            const float deltaRadius = (event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? -1.f : 1.f) *
                                      UNITS_PER_MOUSE_CLICK * event.wheel.y;
            scene.getCamera().approach(deltaRadius);
            break;
        }
        default: {
            // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
            break;
        }
    }
}

void handleUpdate(gfx_testing::game::GameContext &, gfx_testing::scene::Scene &scene) {
    scene.update();
    scene.draw();
}

int main() {
    const gfx_testing::sdl::SdlContext sdlContext;
    gfx_testing::util::ResourceLoader resourceLoader{sdlContext};
    gfx_testing::game::GameContext gameContext(sdlContext, resourceLoader);

    gfx_testing::scene::Scene scene(gameContext);

    auto eventFunction = [&](auto const &event) { handleEvent(gameContext, scene, event); };
    auto updateFunction = [&] { handleUpdate(gameContext, scene); };
    SDL_Log("Begin main loop");
    gameContext.runMainLoop(eventFunction, updateFunction);
    return 0;
}
