#include <ecs/ecs.hpp>
#include <ecs/render_ecs.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <pipeline/gfx/pipeline_definition.hpp>
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

    static ecs::EntityId createMeshObject(game::GameContext &gameContext, io::MeshObjectDef const &def) {
        auto const pipelineNameOpt = pipeline::gfx::parsePipelineName(def.mPipeline);
        CHECK(pipelineNameOpt.has_value()) << "Unknown pipeline name: " << def.mPipeline;
        auto const pipelineName = *pipelineNameOpt;

        auto const attribTreatment = def.mUntextured ? UNTEXTURED_ATTRIB_TREATMENT : util::AttribTreatment{};
        auto const shaderObject = gameContext.mResourceLoader.loadGltfModel(def.mModel, attribTreatment);

        auto transform = glm::translate(glm::mat4(1.0f), def.mPosition);
        if (def.mScale != glm::vec3(1, 1, 1)) {
            transform = glm::scale(transform, def.mScale);
        }

        return ecs::render::createAndEmplaceRenderObjectDynamic(pipelineName, gameContext.getEcs(), def.mName.c_str(),
                                                                gameContext, shaderObject, pipelineName, transform);
    }

    static ecs::EntityId findFirstBehaviorMesh(io::SceneDefinition const &sceneDef, game::GameContext &gameContext) {
        for (auto const &objDef: sceneDef.mObjects) {
            if (auto const *mesh = std::get_if<io::MeshObjectDef>(&objDef)) {
                if (!mesh->mBehavior.empty()) {
                    auto entityId = createMeshObject(gameContext, *mesh);
                    if (mesh->mBehavior == "rotate_z") {
                        entityId.emplace<RotateBehavior>(RotateBehavior{
                                .mAxis = glm::vec3(0, 0, 1), .mRadsPerSecond = mesh->mBehaviorRadsPerSecond});
                    }
                    return entityId;
                }
            }
        }
        // No behavior mesh found — create a placeholder
        return gameContext.getEcs().create("_placeholder");
    }

    SceneObjects::SceneObjects(game::GameContext &gameContext, std::string const &sceneFilename) :
        mGameContext(gameContext), mSceneDefinition(gameContext.mResourceLoader.loadSceneDefinition(sceneFilename)),
        mPropObjects(findFirstBehaviorMesh(mSceneDefinition, gameContext)) {

        auto &ecs = gameContext.getEcs();

        for (auto const &objDef: mSceneDefinition.mObjects) {
            if (auto const *mesh = std::get_if<io::MeshObjectDef>(&objDef)) {
                // Skip the first behavior mesh — already created above
                if (!mesh->mBehavior.empty() && mesh->mName == mPropObjects.getName()) {
                    continue;
                }
                auto entityId = createMeshObject(gameContext, *mesh);
                if (!mesh->mBehavior.empty() && mesh->mBehavior == "rotate_z") {
                    entityId.emplace<RotateBehavior>(RotateBehavior{.mAxis = glm::vec3(0, 0, 1),
                                                                    .mRadsPerSecond = mesh->mBehaviorRadsPerSecond});
                }
            } else if (auto const *skybox = std::get_if<io::SkyboxDef>(&objDef)) {
                SkyBox::create(ecs, gameContext, gameContext.mResourceLoader.loadCubeMap(skybox->mCubemap));
            } else if (auto const *lights = std::get_if<io::PointLightsDef>(&objDef)) {
                auto const shaderObject = PointLight::loadShaderObject(gameContext.mResourceLoader);
                for (auto i = 0; i < lights->mCount; ++i) {
                    auto const phase = (2 * glm::pi<float>() * static_cast<float>(i)) / lights->mCount;
                    PointLight::create(ecs, gameContext, shaderObject, lights->mOrbitRadius, phase);
                }
            }
        }

        // Debug axes (dev tool, always hard-coded)
        DebugAxes::create(ecs, gameContext);
    }

    void SceneObjects::update() const {
        auto &ecs = mGameContext.getEcs();
        auto const deltaTime = mGameContext.getFrameSnapshot().mDeltaTime;

        // Update all entities with RotateBehavior
        auto const rotateView = ecs.mRegistry.view<RotateBehavior, RenderObject>();
        rotateView.each([&](RotateBehavior const &behavior, RenderObject &renderObject) {
            renderObject.mTransform =
                    glm::rotate(renderObject.mTransform, deltaTime * behavior.mRadsPerSecond, behavior.mAxis);
        });

        // Needs to come after the prop objects update
        util::if_present(mDebugNormals, [](const DebugNormals &n) { n.update(); });

        auto const lightView = ecs.mRegistry.view<PointLight>();
        lightView.each([&](PointLight &light) { light.update(); });
    }

    void SceneObjects::toggleDebugNormals(bool enable) {
        if (enable) {
            if (mDebugNormals.has_value()) {
                return;
            }
            mDebugNormals = DebugNormals::create(mPropObjects, mGameContext, {});
        } else {
            if (!mDebugNormals.has_value()) {
                return;
            }

            mDebugNormals->get().mEntityId.destroy();
            mDebugNormals.reset();
        }
    }
} // namespace gfx_testing::render
