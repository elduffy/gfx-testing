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

    template<typename Fn>
    void runEventLoop(Fn &fn) {
        bool quit = false;
        SDL_Event event;

        while (!quit) {
            while (SDL_PollEvent(&event)) {
                fn(event);
                switch (event.type) {
                    case SDL_EVENT_QUIT: {
                        quit = true;
                        SDL_Log("Quitting.");
                        break;
                    }
                    default: {
                        // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
                        break;
                    }
                }
                if (quit) {
                    break;
                }
            }
        }
    }
}
