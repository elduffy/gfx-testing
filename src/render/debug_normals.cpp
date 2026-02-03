#include <ecs/render_ecs.hpp>
#include <render/debug_normals.hpp>

namespace gfx_testing::render {
    struct VertexBufferOffsets {
        // This gets around a lack of offsetof in HLSL https://github.com/microsoft/hlsl-specs/issues/257
        uint32_t mNormalOffset;
        uint32_t mColorOffset;
    };
    static constexpr VertexBufferOffsets VERTEX_BUFFER_OFFSETS{
            .mNormalOffset = offsetof(shader::VertexData, mNormal),
            .mColorOffset = offsetof(shader::VertexData, mColor),
    };

    shader::GpuShaderObject createGpuShaderObject(const game::GameContext &gameContext,
                                                  RenderObject const &targetObject,
                                                  DebugNormals::Options const &options) {
        shader::GpuShaderObject gpuShaderObject(gameContext.mSdlContext);
        gpuShaderObject.reallocVertexBuffer(targetObject.getVertexCount() * 2,
                                            SDL_GPU_BUFFERUSAGE_VERTEX | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE |
                                                    SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ);

        auto &pipeline = gameContext.mPipelines.get(pipeline::compute::PipelineName::DebugNormals);

        auto *commandBuffer = SDL_AcquireGPUCommandBuffer(gameContext.mSdlContext.mDevice);
        CHECK_NE(commandBuffer, nullptr) << "Failed to acquire command buffer: " << SDL_GetError();
        auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);

        CHECK_NE(*gpuShaderObject.mVertexBuffer, nullptr) << "No vertex buffer allocated";
        const SDL_GPUStorageBufferReadWriteBinding rwBinding{
                .buffer = *gpuShaderObject.mVertexBuffer,
                .cycle = false,
        };
        SDL_GPUComputePass *computePass = SDL_BeginGPUComputePass(commandBuffer, nullptr, 0, &rwBinding, 1);
        SDL_BindGPUComputePipeline(computePass, *pipeline.mSdlPipeline);

        SDL_PushGPUComputeUniformData(commandBuffer,
                                      spirv_header_gen::generated::debug_normals_comp::UBO_Options.mBinding, &options,
                                      sizeof(options));

        SDL_PushGPUComputeUniformData(commandBuffer,
                                      spirv_header_gen::generated::debug_normals_comp::UBO_VertexBufferOffsets.mBinding,
                                      &VERTEX_BUFFER_OFFSETS, sizeof(VERTEX_BUFFER_OFFSETS));

        static_assert(spirv_header_gen::generated::debug_normals_comp::SSBO_VertexBufferIn.mReadOnly);
        auto *sourceBuffer = *targetObject.getGpuShaderObject().mVertexBuffer;
        SDL_BindGPUComputeStorageBuffers(computePass,
                                         spirv_header_gen::generated::debug_normals_comp::SSBO_VertexBufferIn.mBinding,
                                         &sourceBuffer, 1);

        const auto groupCount = static_cast<uint32_t>(std::ceil(static_cast<double>(targetObject.getVertexCount()) /
                                                                pipeline.mDefinition.mShader.mWorkgroupSize[0]));
        SDL_DispatchGPUCompute(computePass, groupCount, 1, 1);
        SDL_EndGPUComputePass(computePass);
        return gpuShaderObject;
    }

    DebugNormals &DebugNormals::create(ecs::EntityId target, game::GameContext const &gameContext,
                                       Options const &options) {
        auto &ecs = target.mEcs;
        auto entityId = ecs.create("DebugNormals");
        auto &debugNormals = entityId.emplace<DebugNormals>(entityId, gameContext, target, options);
        entityId.setParent(target);
        return debugNormals;
    }

    DebugNormals::DebugNormals(ecs::EntityId entityId, game::GameContext const &gameContext, ecs::EntityId target,
                               Options const &options) : mEntityId(entityId) {
        auto &targetRo = target.get<RenderObject>();
        ecs::render::emplaceRenderObject<pipeline::gfx::PipelineName::Lines>(
                entityId, createGpuShaderObject(gameContext, targetRo, options), pipeline::gfx::PipelineName::Lines,
                targetRo.mTransform);
    }

    void DebugNormals::update() const {
        const auto targetId = mEntityId.getParent();
        CHECK(targetId.has_value()) << "DebugNormals::update: entity has no parent.";
        const auto &targetObj = targetId->get<RenderObject>();

        mEntityId.get<RenderObject>().mTransform = targetObj.mTransform;
    }
} // namespace gfx_testing::render
