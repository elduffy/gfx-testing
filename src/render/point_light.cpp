#include <ecs/render_ecs.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <io/obj_loader.hpp>
#include <render/point_light.hpp>

namespace gfx_testing::render {
    glm::vec3 getPosition(uint64_t time, float pathRadius, float phase) {
        auto const totalFloatSecs = static_cast<float>(time) / 1000.f;
        auto const theta = phase - 0.5 * totalFloatSecs;
        constexpr auto AMPLITUDE = 2.f;
        return {pathRadius * cos(theta), pathRadius * sin(theta), AMPLITUDE * cos(2 * theta)};
    }

    shader::ShaderObject PointLight::loadShaderObject(io::ResourceLoader const &resourceLoader) {
        return resourceLoader.loadObjModel("uv-sphere.obj", {util::NormalTreatment::AVERAGE});
    }

    PointLight &PointLight::create(ecs::Ecs &ecs, game::GameContext &gameContext,
                                   shader::ShaderObject const &shaderObject, float pathRadius, float phase) {
        auto entityId = ecs.create("PointLight");
        return entityId.emplace<PointLight>(entityId, gameContext, shaderObject, pathRadius, phase);
    }

    PointLight::PointLight(ecs::EntityId entityId, game::GameContext &gameContext,
                           shader::ShaderObject const &shaderObject, float pathRadius, float phase) :
        mGameContext(gameContext),
        mPosWs(getPosition(mGameContext.getFrameSnapshot().mAccumulatedTime, pathRadius, phase)),
        mPathRadius(pathRadius), mPhase(phase),
        mRenderObject(ecs::render::emplaceRenderObject<pipeline::gfx::PipelineName::SimpleColor>(
                entityId, gameContext, shaderObject, pipeline::gfx::PipelineName::SimpleColor,
                translate(glm::mat4(1.0f), mPosWs))) {}

    void PointLight::update() {
        mPosWs = getPosition(mGameContext.getFrameSnapshot().mAccumulatedTime, mPathRadius, mPhase);
        mRenderObject.mTransform = glm::translate(glm::mat4(1.0f), mPosWs);
    }
} // namespace gfx_testing::render
