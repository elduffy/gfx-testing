#pragma once

#include <algorithm>
#include <util.hpp>
#include <boost/scope/scope_exit.hpp>
#include <SDL3/SDL.h>

namespace gfx_testing::sdl {
    class SdlContext {
    public:
        static constexpr util::Extent2D INITIAL_EXTENT = {768, 512};

        explicit SdlContext(bool gfxDebug = true);

        ~SdlContext();

        SDL_Window *mWindow;
        SDL_GPUDevice *mDevice;
    };

    [[nodiscard]] inline auto scopedSubmitCommandBuffer(SDL_GPUCommandBuffer *commandBuffer) {
        boost::scope::scope_exit guard([commandBuffer] {
            SDL_SubmitGPUCommandBuffer(commandBuffer);
        });
        return std::move(guard);
    }


    class SdlShader {
    public:
        SdlShader(SdlContext const &context, SDL_GPUShader *shader);

        ~SdlShader();

        SDL_GPUShader *operator*() const { return mShader; }

        static SdlShader loadShader(
            SdlContext const &context,
            const std::filesystem::path &shaderSourcePath,
            uint32_t samplers,
            uint32_t uniformBuffers,
            uint32_t storageBuffers,
            uint32_t storageTextures
        );

        SdlContext const &mContext;
        SDL_GPUShader *mShader = nullptr;
    };

    class SdlGfxPipeline {
    public:
        SdlGfxPipeline(SdlContext const &context, SDL_GPUGraphicsPipeline *pipeline);

        ~SdlGfxPipeline();

        SDL_GPUGraphicsPipeline *operator*() const { return mPipeline; }

        SdlContext const &mContext;
        SDL_GPUGraphicsPipeline *mPipeline = nullptr;
    };

    class SdlGpuBuffer {
    public:
        SdlGpuBuffer(SdlContext const &context, SDL_GPUBuffer *buffer);

        ~SdlGpuBuffer();

        SDL_GPUBuffer *operator*() const { return mBuffer; }

        SdlContext const &mContext;
        SDL_GPUBuffer *mBuffer = nullptr;
    };

    class SdlGpuTexture {
    public:
        SdlGpuTexture(SdlContext const &context, SDL_GPUTexture *texture);

        ~SdlGpuTexture();

        SDL_GPUTexture *operator*() const { return mTexture; }

        SdlContext const &mContext;
        SDL_GPUTexture *mTexture = nullptr;
    };

    class SdlMappedTransferBuffer {
    public:
        SdlMappedTransferBuffer(SdlContext const &context, SDL_GPUTransferBuffer *buffer, uint8_t *mappedMemory);

        ~SdlMappedTransferBuffer();

        template<typename T>
        T *get(const size_t offset = 0) const { return reinterpret_cast<T *>(mMappedMemory + offset); }

        SdlContext const &mContext;
        SDL_GPUTransferBuffer *mBuffer = nullptr;
        uint8_t *mMappedMemory = nullptr;
    };

    class SdlTransferBuffer {
    public:
        SdlTransferBuffer(SdlContext const &context, SDL_GPUTransferBuffer *buffer);

        ~SdlTransferBuffer();

        SDL_GPUTransferBuffer *operator*() const { return mBuffer; }

        [[nodiscard]] SdlMappedTransferBuffer map(bool cycle) const;

        SdlContext const &mContext;
        SDL_GPUTransferBuffer *mBuffer = nullptr;
    };
}
