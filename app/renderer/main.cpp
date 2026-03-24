#include <debug/imgui_context.hpp>
#include <game.hpp>
#include <io/input_manager.hpp>
#include <memory>
#include <render/draw_context.hpp>
#include <render/scene.hpp>
#include <sdl.hpp>
#include <util/debug.hpp>

#include "cli.hpp"

void handleUpdate(gfx_testing::game::GameContext &gameContext, gfx_testing::render::Scene &scene,
                  gfx_testing::imgui::ImGuiContext *imGuiContext) {
    scene.update();
    auto const &sdlContext = gameContext.mSdlContext;
    const gfx_testing::render::DrawContext drawContext =
            sdlContext.isHeadless() ? gfx_testing::render::DrawContext{sdlContext, gfx_testing::sdl::SdlContext::INITIAL_EXTENT}
                                   : gfx_testing::render::DrawContext{sdlContext};
    if (!drawContext.hasColorTarget()) {
        SDL_Log("Swapchain texture is null");
        return;
    }
    if (!sdlContext.isHeadless()) {
        gameContext.maybeLimitFps();
    }
    scene.onResize(drawContext.mExtent);
    scene.draw(drawContext);
    if (imGuiContext) {
        imGuiContext->renderFrame(drawContext, scene);
    }
}

int main(int argc, char *argv[]) {
    gfx_testing::game::GameSettings gameSettings;
    {
        gfx_testing::Cli cli(argc, argv);
        gameSettings = cli.loadGameSettings();
    }

    std::unique_ptr<gfx_testing::sdl::SdlContext const> sdlContextPtr;
    if (gameSettings.mHeadless) {
        sdlContextPtr = std::make_unique<gfx_testing::sdl::SdlContext>(gfx_testing::util::DEBUG_MODE,
                                                                       SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM);
    } else {
        sdlContextPtr = std::make_unique<gfx_testing::sdl::SdlContext>(
                gfx_testing::util::DEBUG_MODE, gameSettings.getPresentModes(), gameSettings.getSwapchainCompositions());
    }
    auto const &sdlContext = *sdlContextPtr;
    const gfx_testing::io::ResourceLoader resourceLoader{sdlContext};

    gfx_testing::game::GameContext gameContext(sdlContext, resourceLoader, gameSettings);

    std::unique_ptr<gfx_testing::imgui::ImGuiContext> imGuiContext;
    if (!sdlContext.isHeadless()) {
        imGuiContext = std::make_unique<gfx_testing::imgui::ImGuiContext>(sdlContext);
    }

    gfx_testing::render::Scene scene(gameContext);

    bool const headless = gameSettings.mHeadless;
    std::optional<uint32_t> const frameCount = headless ? std::optional(gameSettings.mFrameCount) : std::nullopt;

    gfx_testing::io::InputManager inputManager{gameContext, scene, imGuiContext.get()};
    auto eventFunction = [&](auto const &event) { inputManager.handleEvent(event); };
    auto updateFunction = [&] { handleUpdate(gameContext, scene, imGuiContext.get()); };

    SDL_Log(headless ? "Begin headless rendering (%u frames)" : "Begin main loop", gameSettings.mFrameCount);
    gameContext.runMainLoop(eventFunction, updateFunction, !headless, frameCount);
    if (headless) {
        SDL_Log("Headless rendering complete.");
    }
    return 0;
}
