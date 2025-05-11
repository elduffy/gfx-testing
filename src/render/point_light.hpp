#pragma once
#include <game.hpp>
#include <render/render_object.hpp>

namespace gfx_testing::render {
    class PointLight {
    public:
        explicit PointLight(game::GameContext &gameContext, float pathRadius, float phase);

        void update();

        game::GameContext &mGameContext;
        glm::vec3 mPosWs;
        float mPathRadius;
        float mPhase;
        RenderObject mRenderObject;
    };
} // namespace gfx_testing::render
