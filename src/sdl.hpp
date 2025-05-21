#pragma once

#include <SDL3/SDL.h>
#include <algorithm>
#include <boost/safe_numerics/checked_default.hpp>
#include <boost/scope/scope_exit.hpp>
#include <util/util.hpp>
#include <vector>

namespace gfx_testing::sdl {
    class SdlContext {
    public:
        NO_COPY_NO_MOVE(SdlContext);

        static constexpr util::Extent2D INITIAL_EXTENT = {1280, 720};

        explicit SdlContext(bool gfxDebug, std::vector<SDL_GPUPresentMode> const &presentModes);

        ~SdlContext();

    private:
        void updateSwapchainParameters(std::vector<SDL_GPUPresentMode> const &presentModes) const;

    public:
        SDL_Window *mWindow;
        SDL_GPUDevice *mDevice;
    };

    [[nodiscard]] inline auto scopedSubmitCommandBuffer(SDL_GPUCommandBuffer *commandBuffer) {
        boost::scope::scope_exit guard([commandBuffer] { SDL_SubmitGPUCommandBuffer(commandBuffer); });
        return std::move(guard);
    }


    class SdlShader {
    public:
        NO_COPY(SdlShader);

        SdlShader(SdlContext const &context, SDL_GPUShader *shader);

        ~SdlShader();

        SdlShader(SdlShader &&other) noexcept : mContext(other.mContext), mShader(other.mShader) {
            other.mShader = nullptr;
        };

        static SdlShader createShader(SdlContext const &context, const uint8_t *code, size_t codeSize,
                                      SDL_GPUShaderStage stage, uint32_t samplers, uint32_t uniformBuffers,
                                      uint32_t storageBuffers, uint32_t storageTextures);

        SDL_GPUShader *operator*() const { return mShader; }

        SdlContext const &mContext;
        SDL_GPUShader *mShader = nullptr;
    };

    class SdlGfxPipeline {
    public:
        NO_COPY(SdlGfxPipeline);

        SdlGfxPipeline(SdlContext const &context, SDL_GPUGraphicsPipeline *pipeline);

        ~SdlGfxPipeline();

        SdlGfxPipeline(SdlGfxPipeline &&other) noexcept : mContext(other.mContext), mPipeline(other.mPipeline) {
            other.mPipeline = nullptr;
        }

        SDL_GPUGraphicsPipeline *operator*() const { return mPipeline; }

        SdlContext const &mContext;
        SDL_GPUGraphicsPipeline *mPipeline = nullptr;
    };

    class SdlGpuBuffer {
    public:
        NO_COPY(SdlGpuBuffer);

        SdlGpuBuffer(SdlContext const &context, SDL_GPUBuffer *buffer);

        SdlGpuBuffer(SdlGpuBuffer &&) noexcept;

        ~SdlGpuBuffer();

        SDL_GPUBuffer *operator*() const { return mBuffer; }

        static SdlGpuBuffer create(SdlContext const &context, SDL_GPUBufferUsageFlags usage, uint32_t size);

        SdlContext const &mContext;
        SDL_GPUBuffer *mBuffer = nullptr;
    };

    class SdlGpuTexture {
    public:
        NO_COPY(SdlGpuTexture);

        SdlGpuTexture(SdlContext const &context, SDL_GPUTexture *texture);

        SdlGpuTexture(SdlGpuTexture &&) noexcept;

        ~SdlGpuTexture();

        SDL_GPUTexture *operator*() const { return mTexture; }

        void reset(SDL_GPUTexture *newTexture);

        SdlContext const &mContext;
        SDL_GPUTexture *mTexture = nullptr;
    };

    class SdlMappedTransferBuffer {
    public:
        NO_COPY_NO_MOVE(SdlMappedTransferBuffer);

        SdlMappedTransferBuffer(SdlContext const &context, SDL_GPUTransferBuffer *buffer, uint8_t *mappedMemory);

        ~SdlMappedTransferBuffer();

        template<typename T>
        T *get(const size_t offset = 0) const {
            return reinterpret_cast<T *>(mMappedMemory + offset);
        }

        SdlContext const &mContext;
        SDL_GPUTransferBuffer *mBuffer = nullptr;
        uint8_t *mMappedMemory = nullptr;
    };

    class SdlTransferBuffer {
    public:
        NO_COPY_NO_MOVE(SdlTransferBuffer);

        SdlTransferBuffer(SdlContext const &context, SDL_GPUTransferBuffer *buffer);

        ~SdlTransferBuffer();

        SDL_GPUTransferBuffer *operator*() const { return mBuffer; }

        [[nodiscard]] SdlMappedTransferBuffer map(bool cycle) const;

        static SdlTransferBuffer create(SdlContext const &context, SDL_GPUTransferBufferUsage usage, uint32_t size);

        SdlContext const &mContext;
        SDL_GPUTransferBuffer *mBuffer = nullptr;
    };

    class SdlSurface {
    public:
        NO_COPY(SdlSurface);

        SdlSurface() = default;
        explicit SdlSurface(SDL_Surface *surface);
        SdlSurface(SdlSurface &&other) noexcept;
        ~SdlSurface();

        SDL_Surface *operator*() const { return mSurface; }
        SdlSurface &operator=(SdlSurface &&other) noexcept;

        util::Extent2D getExtent() const {
            assert(mSurface);
            return {
                    .mWidth = boost::safe_numerics::checked::cast<uint32_t>(mSurface->w),
                    .mHeight = boost::safe_numerics::checked::cast<uint32_t>(mSurface->h),
            };
        }

        SDL_Surface *mSurface = nullptr;
    };

    class SdlGpuSampler {
    public:
        NO_COPY(SdlGpuSampler);

        SdlGpuSampler(SdlContext const &context, SDL_GPUSampler *sampler);
        SdlGpuSampler(SdlGpuSampler &&other) noexcept;
        ~SdlGpuSampler();

        SDL_GPUSampler *operator*() const { return mSampler; }

        SdlContext const &mContext;
        SDL_GPUSampler *mSampler = nullptr;
    };
} // namespace gfx_testing::sdl
