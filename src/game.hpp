#pragma once

#include <sdl.hpp>
#include <chrono>
#include <pipelines.hpp>

namespace gfx_testing::game {
    class GameContext {
    public:
        GameContext(sdl::SdlContext const &sdlContext, std::string const &projectRoot);

        template<typename EventFn, typename UpdateFn>
        void runMainLoop(EventFn &eventFn, UpdateFn &updateFn) {
            SDL_Event event;

            while (true) {
                const auto previousFrameStart = mFrameStart;
                mFrameStart = getTime();
                mDeltaTime = static_cast<float>(mFrameStart - previousFrameStart) / 1000.f;

                while (SDL_PollEvent(&event)) {
                    eventFn(event);
                    switch (event.type) {
                        case SDL_EVENT_QUIT: {
                            SDL_Log("Quitting.");
                            return;
                        }
                        case SDL_EVENT_WINDOW_RESIZED: {
                            break;
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

    private:
        [[nodiscard]] uint64_t getTime() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - mStartTime).count();
        }

    public:
        sdl::SdlContext const &mSdlContext;
        pipeline::Pipelines mPipelines;
        std::chrono::high_resolution_clock::time_point mStartTime = std::chrono::high_resolution_clock::now();
        uint64_t mFrameStart = 0;
        float mDeltaTime = 0.0f;
    };
}
