#include <point_light.hpp>
#include <obj_loader.hpp>

#include <glm/ext/matrix_transform.hpp>

namespace gfx_testing::render {
    PointLight::PointLight(game::GameContext &gameContext, glm::vec3 const &initialPosition):
        mGameContext(gameContext),
        mPosWs(initialPosition),
        mPathRadius(length(initialPosition)),
        mRenderObject(gameContext,
                      gameContext.mResourceLoader.loadObjModel("uv-sphere.obj", model::NormalTreatment::AVERAGE),
                      pipeline::PipelineName::SimpleColor,
                      translate(glm::mat4(1.0f), mPosWs)) {
    }

    void PointLight::update() {
        auto const totalFloatSecs = static_cast<float>(mGameContext.getFrameSnapshot().mAccumulatedTime) / 1000.f;
        auto const theta = -0.5 * totalFloatSecs;
        constexpr auto AMPLITUDE = 2.f;
        mPosWs = {mPathRadius * cos(theta), mPathRadius * sin(theta), AMPLITUDE * cos(2 * theta)};
        mRenderObject.mTransform = glm::translate(glm::mat4(1.0f), mPosWs);
    }
}
