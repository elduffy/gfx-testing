#include <sdl.hpp>

#include <stdexcept>

namespace gfx_testing::sdl {
    SdlContext::SdlContext(const bool gfxDebug) :
        mWidth(768), mHeight(512), mWindow(nullptr), mDevice(nullptr) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
            throw std::runtime_error("Failed to initialize SDL");
        }
        SDL_Log("SDL initialized.");

        mWindow = SDL_CreateWindow("gfx-testing", mWidth, mHeight, SDL_WINDOW_RESIZABLE);
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

    SdlGpuBuffer::~SdlGpuBuffer() {
        SDL_ReleaseGPUBuffer(mContext.mDevice, mBuffer);
    }
}

namespace gfx_testing::sdl {
    SdlMappedTransferBuffer::SdlMappedTransferBuffer(SdlContext const &context,
                                                     SDL_GPUTransferBuffer *buffer,
                                                     void *mappedMemory) :
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
        return {mContext, mBuffer, SDL_MapGPUTransferBuffer(mContext.mDevice, mBuffer, cycle)};
    }
}

namespace gfx_testing::sdl {
    SdlCommandBuffer::SdlCommandBuffer(SDL_GPUCommandBuffer *buffer):
        mBuffer(buffer) {
    }

    SdlCommandBuffer::~SdlCommandBuffer() {
        SDL_SubmitGPUCommandBuffer(mBuffer);
    }
}
