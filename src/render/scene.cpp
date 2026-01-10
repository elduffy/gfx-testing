#include <absl/log/check.h>
#include <array>
#include <boost/scope/scope_exit.hpp>
#include <cmath>
#include <ecs/render_ecs.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <pipeline/pipelines.hpp>
#include <render/point_light.hpp>
#include <render/samplers.hpp>
#include <render/scene.hpp>
#include <sdl_factories.hpp>
#include <tiny_obj_loader.h>
#include <util/optional.hpp>
#include <util/util.hpp>


namespace gfx_testing::render {

    static constexpr glm::vec3 INITIAL_CAMERA_POSITION(5, 5, 5);

    glm::mat4x4 getProjection(const util::Extent2D extent) {
        auto const aspect = static_cast<float>(extent.mWidth) / static_cast<float>(extent.mHeight);
        return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
    }

    Scene::Scene(game::GameContext &gameContext) :
        mGameContext(gameContext), mViewportExtent(sdl::SdlContext::INITIAL_EXTENT), mCamera(INITIAL_CAMERA_POSITION),
        mProjection(getProjection(mViewportExtent)), mSceneObjects(gameContext),
        mDepthTexture(createDepthTexture(gameContext.mSdlContext, sdl::SdlContext::INITIAL_EXTENT)),
        mMultisampleTextureOpt(createMultisampleTexture(gameContext.mSdlContext, sdl::SdlContext::INITIAL_EXTENT)) {}

    void Scene::onResize(const util::Extent2D extent) {
        mViewportExtent = extent;
        mProjection = getProjection(mViewportExtent);
        mDepthTexture = createDepthTexture(mGameContext.mSdlContext, extent);
        if (mMultisampleTextureOpt.has_value()) {
            mMultisampleTextureOpt = createMultisampleTexture(mGameContext.mSdlContext, extent);
        }
    }

    void Scene::update() { mSceneObjects.update(); }

    void Scene::draw(DrawContext const &drawContext) const {
        CHECK_NE(drawContext.mSwapchainTexture, nullptr) << "Scene::draw called with null swapchain.";

        SDL_GPUColorTargetInfo mainColorTarget{
                .texture = drawContext.mSwapchainTexture,
                .clear_color = {0, 0, 0, 1},
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
                .resolve_texture = nullptr,
        };
        if (mMultisampleTextureOpt.has_value()) {
            mainColorTarget.texture = *mMultisampleTextureOpt.value();
            mainColorTarget.store_op = SDL_GPU_STOREOP_RESOLVE;
            mainColorTarget.resolve_texture = drawContext.mSwapchainTexture;
        }

        const SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo{
                .texture = *mDepthTexture,
                .clear_depth = 1.f,
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
                .stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
                .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
                .cycle = true,
        };
        SDL_GPURenderPass *renderPass =
                SDL_BeginGPURenderPass(*drawContext.mCommandBuffer, &mainColorTarget, 1, &depthStencilTargetInfo);
        drawObjects(*drawContext.mCommandBuffer, renderPass);
        SDL_EndGPURenderPass(renderPass);
    }

    std::vector<glm::vec3> getLightPositions(ecs::Ecs const &ecs) {
        auto const view = ecs.mRegistry.view<PointLight>();
        std::vector<glm::vec3> positions;
        std::ranges::transform(view, std::back_inserter(positions),
                               [&](auto &entity) { return view.get<PointLight>(entity).mPosWs; });
        return positions;
    }

    void Scene::drawObjects(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass) const {
        const auto &ecs = mGameContext.getEcs();
        const std::vector<glm::vec3> lightPosWs = getLightPositions(ecs);
        auto const view = mCamera.computeViewMatrix();

        for (auto const &pipeline: pipeline::gfx::ALL_PIPELINES) {
            switch (pipeline.mName) {
                case pipeline::gfx::PipelineName::SimpleColor:
                    drawForPipeline<pipeline::gfx::PipelineName::SimpleColor>(commandBuffer, renderPass, view,
                                                                              lightPosWs);
                    break;
                case pipeline::gfx::PipelineName::Gooch:
                    drawForPipeline<pipeline::gfx::PipelineName::Gooch>(commandBuffer, renderPass, view, lightPosWs);
                    break;
                case pipeline::gfx::PipelineName::Textured:
                    drawForPipeline<pipeline::gfx::PipelineName::Textured>(commandBuffer, renderPass, view, lightPosWs);
                    break;
                case pipeline::gfx::PipelineName::Lines:
                    drawForPipeline<pipeline::gfx::PipelineName::Lines>(commandBuffer, renderPass, view, lightPosWs);
                    break;
                case pipeline::gfx::PipelineName::Lambert:
                    drawForPipeline<pipeline::gfx::PipelineName::Lambert>(commandBuffer, renderPass, view, lightPosWs);
                    break;
                case pipeline::gfx::PipelineName::Skybox:
                    drawForPipeline<pipeline::gfx::PipelineName::Skybox>(commandBuffer, renderPass, view, lightPosWs);
                    break;
            }
        }
    }

    template<pipeline::gfx::PipelineName pipelineName>
    void Scene::drawForPipeline(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass,
                                glm::mat4x4 const &view, std::vector<glm::vec3> const &lightPosWs) const {
        auto const &pipeline = mGameContext.mPipelines.get(pipelineName);
        SDL_BindGPUGraphicsPipeline(renderPass, *pipeline.mSdlPipeline);
        pipeline.bindStorageBuffers(renderPass);

        ecs::render::eachRenderObject<pipelineName>(mGameContext.getEcs(), [&](RenderObject const &renderObject) {
            renderObject.pushPerObjectUniforms(pipeline.mDefinition, commandBuffer, mProjection, view, lightPosWs,
                                               mCamera.getPosition());
            renderObject.render(renderPass);
        });
    }
} // namespace gfx_testing::render
