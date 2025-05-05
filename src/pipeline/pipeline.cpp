//
// Created by eric on 5/3/25.
//

#include <pipeline/pipeline.hpp>

#include <algorithm>
#include <memory_resource>
#include <SDL3/SDL_gpu.h>

#include "pipeline_definition.hpp"
#include <sdl.hpp>
#include <shader_models.hpp>

namespace gfx_testing::pipeline {

    template<typename Params>
    void uploadParams(std::vector<sdl::SdlGpuBuffer> &bufferOut, sdl::SdlContext const &context, Params const &params) {
        constexpr auto dataSize = sizeof(params);
        const auto &buffer = bufferOut.emplace_back(
                sdl::SdlGpuBuffer::create(context, SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ, dataSize));

        auto *commandBuffer = SDL_AcquireGPUCommandBuffer(context.mDevice);
        if (commandBuffer == nullptr) {
            throw std::runtime_error("Failed to acquire GPU command buffer");
        }
        auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);

        auto *copyPass = SDL_BeginGPUCopyPass(commandBuffer);
        const sdl::SdlTransferBuffer transferBuffer = sdl::SdlTransferBuffer::create(
                context, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, dataSize);
        // Write to transfer buffer
        {
            const auto mappedBuffer = transferBuffer.map(false);
            *mappedBuffer.get<Params>() = params;
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
    }

    std::vector<sdl::SdlGpuBuffer> allocateStorageBuffers(sdl::SdlContext const &context,
                                                          PipelineDefinition const &pipelineDefinition) {
        std::vector<sdl::SdlGpuBuffer> result;
        switch (pipelineDefinition.mName) {
            case PipelineName::Gooch: {
                constexpr shader::GoochParams params{
                        .mCoolColor = {0, 0, 0.55},
                        .mWarmColor = {0.3, 0.3, 0},
                };
                uploadParams(result, context, params);
                break;
            }
            case PipelineName::Lambert: {
                constexpr shader::LambertParams params{
                        .mUnlitColor = {.1, .1, .1},
                        .mLitColor = {1, 1, 1},
                };
                uploadParams(result, context, params);
                break;
            }
            case PipelineName::SimpleColor:
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
