#include <SDL3/SDL.h>
#include <game.hpp>
#include <imgui_context.hpp>
#include <render/scene.hpp>
#include <sdl.hpp>
#include <util/util.hpp>

void handleEvent(gfx_testing::game::GameContext &gameContext, gfx_testing::render::Scene &scene,
                 gfx_testing::imgui::ImGuiContext &imGuiContext, SDL_Event const &event) {
    if (imGuiContext.processEvent(event)) {
        return;
    }
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
            if (!event.key.down && !event.key.repeat && event.key.key == SDLK_D) {
                imGuiContext.toggleOpen();
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

void handleUpdate(gfx_testing::game::GameContext &, gfx_testing::render::Scene &scene) {
    scene.update();
    scene.draw();
}

int main() {
    constexpr auto DEBUG_MODE = true;

    constexpr gfx_testing::game::GameSettings gameSettings{.mTargetFps = 120.f};
    std::vector<SDL_GPUPresentMode> presentModes;
    if constexpr (gameSettings.mTargetFps.has_value()) {
        presentModes.push_back(SDL_GPU_PRESENTMODE_MAILBOX);
        presentModes.push_back(SDL_GPU_PRESENTMODE_IMMEDIATE);
    }
    presentModes.push_back(SDL_GPU_PRESENTMODE_VSYNC);

    const gfx_testing::sdl::SdlContext sdlContext{DEBUG_MODE, presentModes};
    const gfx_testing::util::ResourceLoader resourceLoader{sdlContext};

    gfx_testing::game::GameContext gameContext(sdlContext, resourceLoader, gameSettings);
    gfx_testing::imgui::ImGuiContext imGuiContext{sdlContext};

    gfx_testing::render::Scene scene(gameContext, imGuiContext);

    auto eventFunction = [&](auto const &event) { handleEvent(gameContext, scene, imGuiContext, event); };
    auto updateFunction = [&] { handleUpdate(gameContext, scene); };
    SDL_Log("Begin main loop");
    gameContext.runMainLoop(eventFunction, updateFunction);
    return 0;
}
