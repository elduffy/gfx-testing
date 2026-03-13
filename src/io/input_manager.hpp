#pragma once

#include <debug/imgui_context.hpp>
#include <SDL3/SDL_events.h>
#include <game.hpp>
#include <render/scene.hpp>
#include <glm/vec2.hpp>
#include <unordered_map>

namespace gfx_testing::io {
    class InputManager {
    public:
        InputManager(game::GameContext &gameContext, render::Scene &scene, imgui::ImGuiContext &imGuiContext) :
            mGameContext(gameContext), mScene(scene), mImGuiContext(imGuiContext) {}
        void handleEvent(SDL_Event const &event);

        game::GameContext &mGameContext;
        render::Scene &mScene;
        imgui::ImGuiContext &mImGuiContext;

        bool mShift{false};
        std::unordered_map<SDL_FingerID, glm::vec2> mFingers;

    private:
        void doPivot(glm::vec2 normalizedDelta);
        void doPan(glm::vec2 normalizedDelta);
        void doZoom(float delta);
    };
} // namespace gfx_testing::io
