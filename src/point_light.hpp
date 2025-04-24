#pragma once
#include <game.hpp>
#include <render_object.hpp>

namespace gfx_testing::render {
    class PointLight {
    public:
        explicit PointLight(game::GameContext &gameContext, std::filesystem::path const &projectRoot,
                            glm::vec3 const &initialPosition);

        void update();

        game::GameContext &mGameContext;
        glm::vec3 mPosWs;
        float mPathRadius;
        RenderObject mRenderObject;
    };
}
