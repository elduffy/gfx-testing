#pragma once

#include <SDL3/SDL.h>

namespace gfx_testing::sdl {
    class SdlContext {
    public:
        explicit SdlContext(bool gfxDebug = true);

        ~SdlContext();

        SDL_Window *mWindow;
        SDL_GPUDevice *mDevice;
    };
}
