#include <sdl.hpp>

#include <stdexcept>
#include <boost/algorithm/string/replace.hpp>

namespace gfx_testing::sdl {


    SdlContext::SdlContext(const bool gfxDebug, bool vsync) :
        mWindow(nullptr), mDevice(nullptr) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
            throw std::runtime_error("Failed to initialize SDL");
        }
        SDL_Log("SDL initialized.");

        mWindow = SDL_CreateWindow("gfx-testing", INITIAL_EXTENT.mWidth, INITIAL_EXTENT.mHeight, SDL_WINDOW_RESIZABLE);
        if (mWindow == nullptr) {
            SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
            throw std::runtime_error("Failed to create SDL window");
        }
        SDL_Log("Window created.");

        mDevice = SDL_CreateGPUDevice(
                SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
                gfxDebug,
                nullptr);
        if (mDevice == nullptr) {
            SDL_Log("Failed to create GPU device: %s", SDL_GetError());
            throw std::runtime_error("Failed to create GPU device");
        }
        SDL_Log("GPU Device created.");

        if (!SDL_ClaimWindowForGPUDevice(mDevice, mWindow)) {
            SDL_Log("Failed to claim window: %s", SDL_GetError());
            throw std::runtime_error("Failed to claim window");
        }

        updateSwapchainParameters(vsync);
    }

    char const *getSwapchainCompositionName(SDL_GPUSwapchainComposition composition) {
        switch (composition) {
            case SDL_GPU_SWAPCHAINCOMPOSITION_SDR:
                return "SDL_GPU_SWAPCHAINCOMPOSITION_SDR";
            case SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR:
                return "SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR";
            case SDL_GPU_SWAPCHAINCOMPOSITION_HDR_EXTENDED_LINEAR:
                return "SDL_GPU_SWAPCHAINCOMPOSITION_HDR_EXTENDED_LINEAR";
            case SDL_GPU_SWAPCHAINCOMPOSITION_HDR10_ST2084:
                return "SDL_GPU_SWAPCHAINCOMPOSITION_HDR10_ST2084";
        }
        throw std::runtime_error("Unknown GPU swapchain composition");
    }

    char const *getPresentModeName(SDL_GPUPresentMode presentMode) {
        switch (presentMode) {
            case SDL_GPU_PRESENTMODE_VSYNC:
                return "SDL_GPU_PRESENTMODE_VSYNC";
            case SDL_GPU_PRESENTMODE_IMMEDIATE:
                return "SDL_GPU_PRESENTMODE_IMMEDIATE";
            case SDL_GPU_PRESENTMODE_MAILBOX:
                return "SDL_GPU_PRESENTMODE_MAILBOX";
        }
        throw std::runtime_error("Unknown GPU present mode");
    }

    void SdlContext::updateSwapchainParameters(bool vsync) const {
        auto presentMode = SDL_GPU_PRESENTMODE_VSYNC;
        if (vsync) {
            if (SDL_WindowSupportsGPUPresentMode(mDevice, mWindow, SDL_GPU_PRESENTMODE_MAILBOX)) {
                presentMode = SDL_GPU_PRESENTMODE_MAILBOX;
            }
        } else {
            if (SDL_WindowSupportsGPUPresentMode(mDevice, mWindow, SDL_GPU_PRESENTMODE_IMMEDIATE)) {
                presentMode = SDL_GPU_PRESENTMODE_IMMEDIATE;
            }
        }
        constexpr auto swapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
        SDL_Log("Swapchain parameters: composition=%s, present mode=%s",
                getSwapchainCompositionName(swapchainComposition), getPresentModeName(presentMode));
        SDL_SetGPUSwapchainParameters(mDevice, mWindow, swapchainComposition, presentMode);
    }

    SdlContext::~SdlContext() {
        SDL_ReleaseWindowFromGPUDevice(mDevice, mWindow);
        SDL_DestroyWindow(mWindow);
        SDL_DestroyGPUDevice(mDevice);
        SDL_Quit();
    }
}

namespace gfx_testing::sdl {
    SdlShader::SdlShader(SdlContext const &context, SDL_GPUShader *shader):
        mContext(context), mShader(shader) {
    }

    SdlShader::~SdlShader() {
        if (mShader) {
            SDL_ReleaseGPUShader(mContext.mDevice, mShader);
        }
    }

    SdlShader SdlShader::createShader(SdlContext const &context, const uint8_t *code, size_t codeSize,
                                      SDL_GPUShaderStage stage,
                                      uint32_t samplers, uint32_t uniformBuffers, uint32_t storageBuffers,
                                      uint32_t storageTextures) {
        const SDL_GPUShaderCreateInfo shaderCreateInfo{
                .code_size = codeSize,
                .code = code,
                .entrypoint = "main",
                .format = SDL_GPU_SHADERFORMAT_SPIRV,
                .stage = stage,
                .num_samplers = samplers,
                .num_storage_textures = storageTextures,
                .num_storage_buffers = storageBuffers,
                .num_uniform_buffers = uniformBuffers,
        };
        SDL_GPUShader *shader = SDL_CreateGPUShader(context.mDevice, &shaderCreateInfo);
        if (shader == nullptr) {
            throw std::runtime_error("Could not create shader.");
        }
        return {context, shader};
    }
}

namespace gfx_testing::sdl {
    SdlGfxPipeline::SdlGfxPipeline(SdlContext const &context, SDL_GPUGraphicsPipeline *pipeline):
        mContext(context), mPipeline(pipeline) {
    }

    SdlGfxPipeline::~SdlGfxPipeline() {
        SDL_ReleaseGPUGraphicsPipeline(mContext.mDevice, mPipeline);
    }
}


namespace gfx_testing::sdl {
    SdlGpuBuffer::SdlGpuBuffer(SdlContext const &context, SDL_GPUBuffer *buffer):
        mContext(context), mBuffer(buffer) {
    }

    SdlGpuBuffer::SdlGpuBuffer(SdlGpuBuffer &&other) noexcept:
        mContext(other.mContext), mBuffer(other.mBuffer) {
        other.mBuffer = nullptr;
    }

    SdlGpuBuffer::~SdlGpuBuffer() {
        SDL_ReleaseGPUBuffer(mContext.mDevice, mBuffer);
    }
}

namespace gfx_testing::sdl {
    SdlGpuTexture::SdlGpuTexture(SdlContext const &context, SDL_GPUTexture *texture):
        mContext(context), mTexture(texture) {
    }

    SdlGpuTexture::SdlGpuTexture(SdlGpuTexture &&other) noexcept:
        mContext(other.mContext), mTexture(other.mTexture) {
        other.mTexture = nullptr;
    }

    SdlGpuTexture::~SdlGpuTexture() {
        SDL_ReleaseGPUTexture(mContext.mDevice, mTexture);
    }

    void SdlGpuTexture::reset(SDL_GPUTexture *newTexture) {
        SDL_ReleaseGPUTexture(mContext.mDevice, mTexture);
        mTexture = newTexture;
    }

}

namespace gfx_testing::sdl {
    SdlMappedTransferBuffer::SdlMappedTransferBuffer(SdlContext const &context,
                                                     SDL_GPUTransferBuffer *buffer,
                                                     uint8_t *mappedMemory) :
        mContext(context), mBuffer(buffer), mMappedMemory(mappedMemory) {
    }

    SdlMappedTransferBuffer::~SdlMappedTransferBuffer() {
        SDL_UnmapGPUTransferBuffer(mContext.mDevice, mBuffer);
    }

    SdlTransferBuffer::SdlTransferBuffer(SdlContext const &context, SDL_GPUTransferBuffer *buffer):
        mContext(context), mBuffer(buffer) {
    }

    SdlTransferBuffer::~SdlTransferBuffer() {
        SDL_ReleaseGPUTransferBuffer(mContext.mDevice, mBuffer);
    }

    SdlMappedTransferBuffer SdlTransferBuffer::map(bool cycle) const {
        return {mContext, mBuffer,
                static_cast<uint8_t *>(SDL_MapGPUTransferBuffer(mContext.mDevice, mBuffer, cycle))};
    }
}

namespace gfx_testing::sdl {
    SdlSurface::SdlSurface(SDL_Surface *surface):
        mSurface(surface) {
    }

    SdlSurface::~SdlSurface() {
        SDL_DestroySurface(mSurface);
    }
}

namespace gfx_testing::sdl {
    SdlGpuSampler::SdlGpuSampler(SdlContext const &context, SDL_GPUSampler *sampler):
        mContext(context), mSampler(sampler) {
    }

    SdlGpuSampler::~SdlGpuSampler() {
        SDL_ReleaseGPUSampler(mContext.mDevice, mSampler);
    }
}
