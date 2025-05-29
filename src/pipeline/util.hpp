#pragma once
#include <sdl.hpp>
#include <vector>

namespace gfx_testing::pipeline {

    template<typename Params>
    void uploadParams(std::vector<sdl::SdlGpuBuffer> &bufferOut, sdl::SdlContext const &context,
                      SDL_GPUBufferUsageFlags usage, Params const &params) {
        constexpr auto dataSize = sizeof(params);
        const auto &buffer = bufferOut.emplace_back(sdl::SdlGpuBuffer::create(context, usage, dataSize));

        auto *commandBuffer = SDL_AcquireGPUCommandBuffer(context.mDevice);
        CHECK_NE(commandBuffer, nullptr) << "Failed to acquire command buffer: " << SDL_GetError();
        auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);

        auto *copyPass = SDL_BeginGPUCopyPass(commandBuffer);
        const sdl::SdlTransferBuffer transferBuffer =
                sdl::SdlTransferBuffer::create(context, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, dataSize);
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
} // namespace gfx_testing::pipeline
