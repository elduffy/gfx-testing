#pragma once

#include <SDL3/SDL_events.h>
#include <game.hpp>
#include <render/scene.hpp>

namespace gfx_testing::io {
    class InputManager {
    public:
        InputManager(game::GameContext &gameContext, render::Scene &scene, imgui::ImGuiContext &imGuiContext) :
            mGameContext(gameContext), mScene(scene), mImGuiContext(imGuiContext) {}
        void handleEvent(SDL_Event const &event) const;

        game::GameContext &mGameContext;
        render::Scene &mScene;
        imgui::ImGuiContext &mImGuiContext;
    };
} // namespace gfx_testing::io
