#pragma once

#include <sdl.hpp>
#include <chrono>
#include <pipelines.hpp>
#include <stopwatch.hpp>

namespace gfx_testing::game {
    class GameContext {
    public:
        GameContext(sdl::SdlContext const &sdlContext, std::string const &projectRoot);

        template<typename EventFn, typename UpdateFn>
        void runMainLoop(EventFn &eventFn, UpdateFn &updateFn) {
            SDL_Event event;

            mStopwatch.resume();
            while (true) {
                mLastFrame.update(mStopwatch);

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

        [[nodiscard]] util::Snapshot const &getFrameSnapshot() const {
            return mLastFrame;
        }

    private:
        util::Snapshot mLastFrame;

    public:
        sdl::SdlContext const &mSdlContext;
        pipeline::Pipelines mPipelines;
        util::Stopwatch mStopwatch{false};
    };
}
