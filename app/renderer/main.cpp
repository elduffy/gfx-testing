#include <color_target.hpp>
#include <debug/imgui_context.hpp>
#include <game.hpp>
#include <io/input_manager.hpp>
#include <render/draw_context.hpp>
#include <render/scene.hpp>
#include <sdl.hpp>
#include <util/debug.hpp>

#include "cli.hpp"

void handleUpdate(gfx_testing::game::GameContext &gameContext, gfx_testing::render::DrawContext const &drawContext,
                  gfx_testing::render::Scene &scene, gfx_testing::imgui::ImGuiContext &imGuiContext) {
    scene.update();
    if (!drawContext.hasColorTarget()) {
        SDL_Log("Swapchain texture is null");
        return;
    }
    gameContext.maybeLimitFps();
    scene.onResize(drawContext.mExtent);
    scene.draw(drawContext);
    imGuiContext.renderFrame(drawContext, scene);
}

gfx_testing::sdl::SdlContext createSdlContext(gfx_testing::game::GameSettings const &gameSettings) {
    if (gameSettings.mHeadless) {
        return gfx_testing::sdl::SdlContext(gfx_testing::util::DEBUG_MODE);
    }
    return gfx_testing::sdl::SdlContext(gfx_testing::util::DEBUG_MODE, gameSettings.getPresentModes(),
                                        gameSettings.getSwapchainCompositions());
}

int main(int argc, char *argv[]) {
    gfx_testing::game::GameSettings gameSettings;
    {
        gfx_testing::Cli cli(argc, argv);
        gameSettings = cli.loadGameSettings();
    }
    auto const sdlContext = createSdlContext(gameSettings);
    gfx_testing::sdl::ColorTargetSource colorTargetSource{sdlContext};
    const gfx_testing::io::ResourceLoader resourceLoader{sdlContext};

    gfx_testing::game::GameContext gameContext(sdlContext, resourceLoader, gameSettings);
    gfx_testing::imgui::ImGuiContext imGuiContext(sdlContext);
    gfx_testing::render::Scene scene(gameContext);
    gfx_testing::io::InputManager inputManager{gameContext, scene, imGuiContext};

    auto eventFunction = [&](auto const &event) { inputManager.handleEvent(event); };
    auto updateFunction = [&] {
        handleUpdate(gameContext, gfx_testing::render::DrawContext{sdlContext, colorTargetSource}, scene, imGuiContext);
    };

    SDL_Log(gameSettings.mHeadless ? "Begin headless rendering (%u frames)" : "Begin main loop",
            gameSettings.mFrameCount);
    gameContext.runMainLoop(eventFunction, updateFunction,
                            /*pollEvents=*/!gameSettings.mHeadless,
                            /*frameCount=*/gameSettings.mHeadless ? std::optional(gameSettings.mFrameCount)
                                                                  : std::nullopt);
    if (gameSettings.mHeadless) {
        SDL_Log("Headless rendering complete.");
    }
    return 0;
}
