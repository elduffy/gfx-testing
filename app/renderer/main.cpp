#include <game.hpp>
#include <imgui_context.hpp>
#include <io/input_manager.hpp>
#include <render/draw_context.hpp>
#include <render/scene.hpp>
#include <sdl.hpp>
#include <util/debug.hpp>
#include <util/util.hpp>

void handleUpdate(gfx_testing::game::GameContext &gameContext, gfx_testing::render::Scene &scene,
                  gfx_testing::imgui::ImGuiContext &imGuiContext) {
    scene.update();
    const gfx_testing::render::DrawContext drawContext{gameContext.mSdlContext};
    if (!drawContext.hasSwapchain()) {
        SDL_Log("Swapchain texture is null");
        return;
    }
    gameContext.maybeLimitFps();
    scene.draw(drawContext);
    imGuiContext.renderFrame(drawContext, scene);
}

int main() {
    constexpr gfx_testing::game::GameSettings gameSettings{.mTargetFps = {120}};
    std::vector<SDL_GPUPresentMode> presentModes;
    if constexpr (gameSettings.mTargetFps.has_value()) {
        presentModes.push_back(SDL_GPU_PRESENTMODE_MAILBOX);
        presentModes.push_back(SDL_GPU_PRESENTMODE_IMMEDIATE);
    }
    presentModes.push_back(SDL_GPU_PRESENTMODE_VSYNC);

    const gfx_testing::sdl::SdlContext sdlContext{gfx_testing::util::DEBUG_MODE, presentModes};
    const gfx_testing::io::ResourceLoader resourceLoader{sdlContext};

    gfx_testing::game::GameContext gameContext(sdlContext, resourceLoader, gameSettings);
    gfx_testing::imgui::ImGuiContext imGuiContext{sdlContext};

    gfx_testing::render::Scene scene(gameContext);
    gfx_testing::io::InputManager inputManager{gameContext, scene, imGuiContext};

    auto eventFunction = [&](auto const &event) { inputManager.handleEvent(event); };
    auto updateFunction = [&] { handleUpdate(gameContext, scene, imGuiContext); };
    SDL_Log("Begin main loop");
    gameContext.runMainLoop(eventFunction, updateFunction);
    return 0;
}
