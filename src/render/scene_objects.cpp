#include <ecs/ecs.hpp>
#include <render/debug_axes.hpp>
#include <render/point_light.hpp>
#include <render/scene_objects.hpp>
#include <render/sky_box.hpp>
#include <util/optional.hpp>

namespace gfx_testing::render {
    static constexpr util::AttribTreatment UNTEXTURED_ATTRIB_TREATMENT{
            .mNormal = util::NormalTreatment::SPLIT,
            .mTexCoord = util::TexCoordTreatment::DISCARD,
    };

    SceneObjects::SceneObjects(game::GameContext &gameContext, ecs::Ecs &ecs) :
        mGameContext(gameContext), mEcs(ecs),
        mPropObjects(ecs.createAndEmplace<RenderObject>(
                gameContext, gameContext.mResourceLoader.loadGltfModel("basic-shapes.glb", UNTEXTURED_ATTRIB_TREATMENT),
                pipeline::gfx::PipelineName::Gooch, translate(glm::mat4(1.0f), PROP_OBJECTS_POSITION))) {
        // Point lights
        constexpr auto NUM_POINT_LIGHTS = 3;
        auto const shaderObject = PointLight::loadShaderObject(gameContext.mResourceLoader);
        for (auto i = 0; i < NUM_POINT_LIGHTS; ++i) {
            auto const phase = (2 * glm::pi<float>() * static_cast<float>(i)) / NUM_POINT_LIGHTS;
            PointLight::create(ecs, gameContext, shaderObject, glm::length(INITIAL_LIGHT_POSITION), phase);
        }
        // Skybox
        SkyBox::create(ecs, gameContext, gameContext.mResourceLoader.loadCubeMap("desert-night"));
        // Debug axes
        DebugAxes::create(ecs, gameContext);
        // Landscape
        ecs.createAndEmplace<RenderObject>(
                gameContext, gameContext.mResourceLoader.loadGltfModel("cube.glb", UNTEXTURED_ATTRIB_TREATMENT),
                pipeline::gfx::PipelineName::Lambert,
                glm::scale(translate(glm::mat4(1.0f), LANDSCAPE_POSITION), LANDSCAPE_SCALE));
        // Textured object
        ecs.createAndEmplace<RenderObject>(
                gameContext, gameContext.mResourceLoader.loadGltfModel("viking-room.glb"),
                pipeline::gfx::PipelineName::Textured,
                glm::scale(translate(glm::mat4(1.0f), TEXTURE_OBJECT_POSITION), TEXTURE_OBJECT_SCALE));
    }

    void SceneObjects::update() const {
        constexpr auto RADS_PER_SECOND = glm::pi<float>() / 8.f;

        auto &propObjects = mPropObjects.mRef;
        propObjects.mTransform =
                rotate(propObjects.mTransform, mGameContext.getFrameSnapshot().mDeltaTime * RADS_PER_SECOND,
                       glm::vec3(0, 0, 1));
        // Needs to come after the prop objects update
        util::if_present(mDebugNormals, [](const DebugNormals &n) { n.update(); });

        auto const view = mEcs.mRegistry.view<PointLight>();
        view.each([&](PointLight &light) { light.update(); });
    }

    void SceneObjects::toggleDebugNormals(bool enable) {
        if (enable) {
            if (mDebugNormals.has_value()) {
                return;
            }
            mDebugNormals = DebugNormals::create(mPropObjects.mId, mGameContext, {});
        } else {
            if (!mDebugNormals.has_value()) {
                return;
            }

            mDebugNormals->get().mEntityId.destroy();
            mDebugNormals.reset();
        }
    }
} // namespace gfx_testing::render
