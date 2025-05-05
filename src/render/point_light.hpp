#pragma once
#include <game.hpp>
#include <render/render_object.hpp>

namespace gfx_testing::render {
    class PointLight {
    public:
        explicit PointLight(game::GameContext &gameContext, glm::vec3 const &initialPosition);

        void update();

        game::GameContext &mGameContext;
        glm::vec3 mPosWs;
        float mPathRadius;
        RenderObject mRenderObject;
    };
} // namespace gfx_testing::render
