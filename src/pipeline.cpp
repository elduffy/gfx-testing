//
// Created by eric on 5/3/25.
//

#include "pipeline.hpp"

#include <algorithm>
#include <memory_resource>
#include <SDL3/SDL_gpu.h>

#include "gooch.frag.hpp"
#include "pipeline_definition.hpp"
#include "sdl.hpp"
#include "shader_models.hpp"

namespace gfx_testing::pipeline {
    std::vector<sdl::SdlGpuBuffer> allocateStorageBuffers(sdl::SdlContext const &context,
                                                          PipelineDefinition const &pipelineDefinition) {
        std::vector<sdl::SdlGpuBuffer> result;
        switch (pipelineDefinition.mName) {
            case PipelineName::Gooch: {
                constexpr shader::GoochParams params{
                        .mCoolColor = {0, 0, 0.55},
                        .mWarmColor = {0.3, 0.3, 0},
                };
                constexpr auto dataSize = sizeof(params);
                const auto &buffer = result.emplace_back(
                        sdl::SdlGpuBuffer::create(context, SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ, dataSize));

                auto *commandBuffer = SDL_AcquireGPUCommandBuffer(context.mDevice);
                if (commandBuffer == nullptr) {
                    throw std::runtime_error("Failed to acquire GPU command buffer");
                }
                auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);

                auto *copyPass = SDL_BeginGPUCopyPass(commandBuffer);
                constexpr SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
                        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                        .size = dataSize,
                };
                const sdl::SdlTransferBuffer transferBuffer{
                        context, SDL_CreateGPUTransferBuffer(context.mDevice, &transferBufferCreateInfo)};
                // Write to transfer buffer
                {
                    const auto mappedBuffer = transferBuffer.map(false);
                    auto *vertexData = mappedBuffer.get<shader::GoochParams>();
                    *vertexData = params;
                }
                const SDL_GPUTransferBufferLocation source = {
                        .transfer_buffer = *transferBuffer,
                        .offset = 0,
                };
                const SDL_GPUBufferRegion destination = {
                        .buffer = *buffer,
                        .offset = 0,
                        .size = dataSize,
                };
                SDL_UploadToGPUBuffer(copyPass, &source, &destination, false);
                SDL_EndGPUCopyPass(copyPass);
                break;
            }
            case PipelineName::Diffuse:
            case PipelineName::Textured:
            case PipelineName::Lines:
                break;
        }
        return result;
    }

    Pipeline::Pipeline(PipelineDefinition const &definition, sdl::SdlGfxPipeline sdlPipeline) :
        mDefinition(definition), mSdlPipeline(std::move(sdlPipeline)),
        mBuffers(allocateStorageBuffers(sdlPipeline.mContext, mDefinition)) {
    }

    void Pipeline::bindStorageBuffers(SDL_GPURenderPass *renderPass) const {
        SDL_BindGPUFragmentStorageBuffers(renderPass, 0, mBuffers.mStorageBufferPtrs.data(),
                                          mBuffers.mStorageBufferPtrs.size());
    }
}
