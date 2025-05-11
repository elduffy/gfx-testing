#include <glm/ext/matrix_transform.hpp>
#include <render/point_light.hpp>
#include <util/obj_loader.hpp>

namespace gfx_testing::render {
    glm::vec3 getPosition(uint64_t time, float pathRadius, float phase) {
        auto const totalFloatSecs = static_cast<float>(time) / 1000.f;
        auto const theta = phase - 0.5 * totalFloatSecs;
        constexpr auto AMPLITUDE = 2.f;
        return {pathRadius * cos(theta), pathRadius * sin(theta), AMPLITUDE * cos(2 * theta)};
    }

    PointLight::PointLight(game::GameContext &gameContext, float pathRadius, float phase) :
        mGameContext(gameContext),
        mPosWs(getPosition(mGameContext.getFrameSnapshot().mAccumulatedTime, pathRadius, phase)),
        mPathRadius(pathRadius), mPhase(phase),
        mRenderObject(gameContext,
                      gameContext.mResourceLoader.loadObjModel("uv-sphere.obj", util::NormalTreatment::AVERAGE),
                      pipeline::PipelineName::SimpleColor, translate(glm::mat4(1.0f), mPosWs)) {}

    void PointLight::update() {
        mPosWs = getPosition(mGameContext.getFrameSnapshot().mAccumulatedTime, mPathRadius, mPhase);
        mRenderObject.mTransform = glm::translate(glm::mat4(1.0f), mPosWs);
    }
} // namespace gfx_testing::render
