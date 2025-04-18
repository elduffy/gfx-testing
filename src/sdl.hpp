#pragma once

#include <SDL3/SDL.h>

namespace gfx_testing::sdl {
    class SdlContext {
    public:
        explicit SdlContext(bool gfxDebug = true);

        ~SdlContext();

        int32_t mWidth;
        int32_t mHeight;
        SDL_Window *mWindow;
        SDL_GPUDevice *mDevice;
    };

    template<typename EventFn, typename UpdateFn>
    void runEventLoop(EventFn &eventFn, UpdateFn &updateFn) {
        SDL_Event event;

        while (true) {
            while (SDL_PollEvent(&event)) {
                eventFn(event);
                switch (event.type) {
                    case SDL_EVENT_QUIT: {
                        SDL_Log("Quitting.");
                        return;
                    }
                    default: {
                        // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
                        break;
                    }
                }
            }
            updateFn();
        }
    }

    class SdlShader {
    public:
        SdlShader(SdlContext const &context, SDL_GPUShader *shader);

        ~SdlShader();

        SDL_GPUShader *operator*() const { return mShader; }

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

    class SdlMappedTransferBuffer {
    public:
        SdlMappedTransferBuffer(SdlContext const &context, SDL_GPUTransferBuffer *buffer, void *mappedMemory);

        ~SdlMappedTransferBuffer();

        template<typename T>
        T *get() const { return static_cast<T *>(mMappedMemory); }

        SdlContext const &mContext;
        SDL_GPUTransferBuffer *mBuffer = nullptr;
        void *mMappedMemory = nullptr;
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

    class SdlCommandBuffer {
    public:
        explicit SdlCommandBuffer(SDL_GPUCommandBuffer *buffer);

        ~SdlCommandBuffer();

        SDL_GPUCommandBuffer *operator*() const { return mBuffer; }

        SDL_GPUCommandBuffer *mBuffer = nullptr;
    };
}
