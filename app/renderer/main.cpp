#include <game.hpp>
#include <imgui_context.hpp>
#include <io/input_manager.hpp>
#include <render/scene.hpp>
#include <sdl.hpp>
#include <util/debug.hpp>
#include <util/util.hpp>

void handleUpdate(gfx_testing::game::GameContext &, gfx_testing::render::Scene &scene) {
    scene.update();
    scene.draw();
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

    gfx_testing::render::Scene scene(gameContext, imGuiContext);
    gfx_testing::io::InputManager inputManager{gameContext, scene, imGuiContext};

    auto eventFunction = [&](auto const &event) { inputManager.handleEvent(event); };
    auto updateFunction = [&] { handleUpdate(gameContext, scene); };
    SDL_Log("Begin main loop");
    gameContext.runMainLoop(eventFunction, updateFunction);
    return 0;
}
