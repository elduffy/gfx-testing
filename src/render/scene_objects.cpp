#include <render/scene_objects.hpp>
#include <util/optional.hpp>

namespace gfx_testing::render {
    static constexpr util::AttribTreatment UNTEXTURED_ATTRIB_TREATMENT{
            .mNormal = util::NormalTreatment::SPLIT,
            .mTexCoord = util::TexCoordTreatment::DISCARD,
    };


    std::vector<PointLight> initPointLights(game::GameContext &gameContext) {
        constexpr auto NUM_POINT_LIGHTS = 3;
        auto const shaderObject = PointLight::loadShaderObject(gameContext.mResourceLoader);
        std::vector<PointLight> pointLights;
        pointLights.reserve(NUM_POINT_LIGHTS);
        for (auto i = 0; i < NUM_POINT_LIGHTS; ++i) {
            auto const phase = (2 * glm::pi<float>() * static_cast<float>(i)) / NUM_POINT_LIGHTS;
            pointLights.emplace_back(gameContext, shaderObject, glm::length(INITIAL_LIGHT_POSITION), phase);
        }
        return pointLights;
    }

    SceneObjects::SceneObjects(game::GameContext &gameContext, ecs::Ecs &ecs) :
        mGameContext(gameContext),
        mSkyBox(SkyBox::create(ecs, gameContext, gameContext.mResourceLoader.loadCubeMap("desert-night"))),
        mPropObjects(gameContext,
                     gameContext.mResourceLoader.loadGltfModel("basic-shapes.glb", UNTEXTURED_ATTRIB_TREATMENT),
                     pipeline::gfx::PipelineName::Gooch, translate(glm::mat4(1.0f), PROP_OBJECTS_POSITION)),
        mLandscape(gameContext, gameContext.mResourceLoader.loadGltfModel("cube.glb", UNTEXTURED_ATTRIB_TREATMENT),
                   pipeline::gfx::PipelineName::Lambert,
                   glm::scale(translate(glm::mat4(1.0f), LANDSCAPE_POSITION), LANDSCAPE_SCALE)),
        mTextureObject(gameContext, gameContext.mResourceLoader.loadGltfModel("viking-room.glb"),
                       pipeline::gfx::PipelineName::Textured,
                       glm::scale(translate(glm::mat4(1.0f), TEXTURE_OBJECT_POSITION), TEXTURE_OBJECT_SCALE)),
        mDebugAxes(gameContext), mPointLights(initPointLights(gameContext)) {

        ecs.addRenderObject(mPropObjects);
        ecs.addRenderObject(mLandscape);
        ecs.addRenderObject(mTextureObject);
        ecs.addRenderObject(mDebugAxes.mRenderObject);
        // TODO: debug normals no longer working
        for (auto const &pl: mPointLights) {
            ecs.addRenderObject(pl.mRenderObject);
        }
    }

    std::vector<util::cref_vec<RenderObject>> SceneObjects::calculateRenderObjectsByPipeline() const {
        std::vector<util::cref_vec<RenderObject>> result{pipeline::gfx::ALL_PIPELINES.size()};

        std::vector<util::cref<RenderObject>> renderObjects{
                mSkyBox.mRenderObject, mDebugAxes.mRenderObject, mPropObjects, mLandscape, mTextureObject,
        };
        util::if_present(mDebugNormals.mRenderObject,
                         [&renderObjects](auto const &v) { renderObjects.emplace_back(v); });
        for (auto const &light: mPointLights) {
            renderObjects.emplace_back(light.mRenderObject);
        }

        for (auto const &objPtr: renderObjects) {
            result.at(pipeline::gfx::getIndex(objPtr.get().getPipelineName())).push_back(objPtr);
        }
        return result;
    }

    void SceneObjects::update() {
        constexpr auto RADS_PER_SECOND = glm::pi<float>() / 8.f;

        mPropObjects.mTransform =
                rotate(mPropObjects.mTransform, mGameContext.getFrameSnapshot().mDeltaTime * RADS_PER_SECOND,
                       glm::vec3(0, 0, 1));
        // Needs to come after the prop objects update
        mDebugNormals.update();
        for (auto &light: mPointLights) {
            light.update();
        }
    }
    void SceneObjects::toggleDebugNormals(bool enable) {
        if (enable) {
            mDebugNormals.enable(mGameContext, mPropObjects, {});
        } else {
            mDebugNormals.disable();
        }
        // mRenderObjectsByPipeline = calculateRenderObjectsByPipeline();
    }
} // namespace gfx_testing::render
