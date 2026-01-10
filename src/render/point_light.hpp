#pragma once
#include <ecs/ecs.hpp>
#include <game.hpp>
#include <render/render_object.hpp>

namespace gfx_testing::render {
    class PointLight {
    public:
        static PointLight &create(ecs::Ecs &ecs, game::GameContext &gameContext,
                                  shader::ShaderObject const &shaderObject, float pathRadius, float phase);
        PointLight(ecs::EntityId entityId, game::GameContext &gameContext, shader::ShaderObject const &shaderObject,
                   float pathRadius, float phase);

        void update();

        static shader::ShaderObject loadShaderObject(io::ResourceLoader const &resourceLoader);

        game::GameContext &mGameContext;
        glm::vec3 mPosWs;
        float mPathRadius;
        float mPhase;
        RenderObject &mRenderObject;
    };
} // namespace gfx_testing::render
