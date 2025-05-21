#include <absl/log/check.h>
#include <boost/algorithm/string/replace.hpp>
#include <sdl.hpp>
#include <stdexcept>

namespace gfx_testing::sdl {


    SdlContext::SdlContext(const bool gfxDebug, std::vector<SDL_GPUPresentMode> const &presentModes) :
        mWindow(nullptr), mDevice(nullptr) {
        CHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) << "Failed to initialize SDL: " << SDL_GetError();
        SDL_Log("SDL initialized.");

        mWindow = SDL_CreateWindow("gfx-testing", INITIAL_EXTENT.mWidth, INITIAL_EXTENT.mHeight, SDL_WINDOW_RESIZABLE);
        CHECK_NE(mWindow, nullptr) << "SDL_CreateWindow failed: " << SDL_GetError();
        SDL_Log("Window created.");

        mDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
                                      gfxDebug, nullptr);
        CHECK_NE(mDevice, nullptr) << "SDL_CreateGPUDevice failed: " << SDL_GetError();
        SDL_Log("GPU Device created.");

        CHECK(SDL_ClaimWindowForGPUDevice(mDevice, mWindow)) << "Failed to claim window: " << SDL_GetError();

        updateSwapchainParameters(presentModes);
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

    void SdlContext::updateSwapchainParameters(std::vector<SDL_GPUPresentMode> const &presentModes) const {
        auto presentMode = SDL_GPU_PRESENTMODE_VSYNC;
        for (auto const &modeToTry: presentModes) {
            if (SDL_WindowSupportsGPUPresentMode(mDevice, mWindow, modeToTry)) {
                presentMode = modeToTry;
                break;
            }
        }
        constexpr auto swapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
        SDL_Log("Swapchain parameters: composition=%s, present mode=%s",
                getSwapchainCompositionName(swapchainComposition), getPresentModeName(presentMode));
        SDL_SetGPUSwapchainParameters(mDevice, mWindow, swapchainComposition, presentMode);
    }

    SdlContext::~SdlContext() {
        SDL_ReleaseWindowFromGPUDevice(mDevice, mWindow);
        SDL_DestroyGPUDevice(mDevice);
        SDL_DestroyWindow(mWindow);
        SDL_Quit();
    }
} // namespace gfx_testing::sdl

namespace gfx_testing::sdl {
    SdlShader::SdlShader(SdlContext const &context, SDL_GPUShader *shader) : mContext(context), mShader(shader) {}

    SdlShader::~SdlShader() {
        if (mShader) {
            SDL_ReleaseGPUShader(mContext.mDevice, mShader);
        }
    }

    SdlShader SdlShader::createShader(SdlContext const &context, const uint8_t *code, size_t codeSize,
                                      SDL_GPUShaderStage stage, uint32_t samplers, uint32_t uniformBuffers,
                                      uint32_t storageBuffers, uint32_t storageTextures) {
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
        CHECK_NE(shader, nullptr) << "SDL_CreateGPUShader failed: " << SDL_GetError();
        return {context, shader};
    }
} // namespace gfx_testing::sdl

namespace gfx_testing::sdl {
    SdlGfxPipeline::SdlGfxPipeline(SdlContext const &context, SDL_GPUGraphicsPipeline *pipeline) :
        mContext(context), mPipeline(pipeline) {}

    SdlGfxPipeline::~SdlGfxPipeline() { SDL_ReleaseGPUGraphicsPipeline(mContext.mDevice, mPipeline); }
} // namespace gfx_testing::sdl


namespace gfx_testing::sdl {
    SdlGpuBuffer::SdlGpuBuffer(SdlContext const &context, SDL_GPUBuffer *buffer) : mContext(context), mBuffer(buffer) {}

    SdlGpuBuffer::SdlGpuBuffer(SdlGpuBuffer &&other) noexcept : mContext(other.mContext), mBuffer(other.mBuffer) {
        other.mBuffer = nullptr;
    }

    SdlGpuBuffer::~SdlGpuBuffer() { SDL_ReleaseGPUBuffer(mContext.mDevice, mBuffer); }

    SdlGpuBuffer SdlGpuBuffer::create(SdlContext const &context, SDL_GPUBufferUsageFlags usage, uint32_t size) {
        const SDL_GPUBufferCreateInfo createInfo = {
                .usage = usage,
                .size = size,
        };
        return {context, SDL_CreateGPUBuffer(context.mDevice, &createInfo)};
    }
} // namespace gfx_testing::sdl

namespace gfx_testing::sdl {
    SdlGpuTexture::SdlGpuTexture(SdlContext const &context, SDL_GPUTexture *texture) :
        mContext(context), mTexture(texture) {}

    SdlGpuTexture::SdlGpuTexture(SdlGpuTexture &&other) noexcept : mContext(other.mContext), mTexture(other.mTexture) {
        other.mTexture = nullptr;
    }

    SdlGpuTexture::~SdlGpuTexture() { SDL_ReleaseGPUTexture(mContext.mDevice, mTexture); }

    void SdlGpuTexture::reset(SDL_GPUTexture *newTexture) {
        SDL_ReleaseGPUTexture(mContext.mDevice, mTexture);
        mTexture = newTexture;
    }

    SdlTransferBuffer SdlTransferBuffer::create(SdlContext const &context, SDL_GPUTransferBufferUsage usage,
                                                uint32_t size) {
        const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{
                .usage = usage,
                .size = size,
        };
        return {context, SDL_CreateGPUTransferBuffer(context.mDevice, &transferBufferCreateInfo)};
    }
} // namespace gfx_testing::sdl

namespace gfx_testing::sdl {
    SdlMappedTransferBuffer::SdlMappedTransferBuffer(SdlContext const &context, SDL_GPUTransferBuffer *buffer,
                                                     uint8_t *mappedMemory) :
        mContext(context), mBuffer(buffer), mMappedMemory(mappedMemory) {}

    SdlMappedTransferBuffer::~SdlMappedTransferBuffer() { SDL_UnmapGPUTransferBuffer(mContext.mDevice, mBuffer); }

    SdlTransferBuffer::SdlTransferBuffer(SdlContext const &context, SDL_GPUTransferBuffer *buffer) :
        mContext(context), mBuffer(buffer) {}

    SdlTransferBuffer::~SdlTransferBuffer() { SDL_ReleaseGPUTransferBuffer(mContext.mDevice, mBuffer); }

    SdlMappedTransferBuffer SdlTransferBuffer::map(bool cycle) const {
        return {mContext, mBuffer, static_cast<uint8_t *>(SDL_MapGPUTransferBuffer(mContext.mDevice, mBuffer, cycle))};
    }
} // namespace gfx_testing::sdl

namespace gfx_testing::sdl {
    SdlSurface::SdlSurface(SDL_Surface *surface) : mSurface(surface) {}

    SdlSurface::SdlSurface(SdlSurface &&other) noexcept : mSurface(other.mSurface) { other.mSurface = nullptr; }

    SdlSurface::~SdlSurface() { SDL_DestroySurface(mSurface); }

    SdlSurface &SdlSurface::operator=(SdlSurface &&other) noexcept {
        mSurface = other.mSurface;
        other.mSurface = nullptr;
        return *this;
    }
} // namespace gfx_testing::sdl

namespace gfx_testing::sdl {
    SdlGpuSampler::SdlGpuSampler(SdlContext const &context, SDL_GPUSampler *sampler) :
        mContext(context), mSampler(sampler) {}

    SdlGpuSampler::SdlGpuSampler(SdlGpuSampler &&other) noexcept : mContext(other.mContext), mSampler(other.mSampler) {
        other.mSampler = nullptr;
    }

    SdlGpuSampler::~SdlGpuSampler() { SDL_ReleaseGPUSampler(mContext.mDevice, mSampler); }
} // namespace gfx_testing::sdl
