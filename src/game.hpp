#pragma once

#include <sdl.hpp>
#include <pipelines.hpp>
#include <stopwatch.hpp>

#include "frame_watcher.hpp"
#include "samplers.hpp"

namespace gfx_testing::game {
    class GameContext {
    public:
        GameContext(sdl::SdlContext const &sdlContext, util::ResourceLoader const &resourceLoader);

        template<typename EventFn, typename UpdateFn>
        void runMainLoop(EventFn &eventFn, UpdateFn &updateFn) {
            SDL_Event event;

            mStopwatch.resume();
            while (true) {
                mLastFrame.update(mStopwatch);
                mFrameWatcher.onFrame();

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
        util::Snapshot mLastFrame{};

    public:
        sdl::SdlContext const &mSdlContext;
        util::FrameWatcher mFrameWatcher;
        util::ResourceLoader const &mResourceLoader;
        pipeline::Pipelines mPipelines;
        render::Samplers mSamplers;
        util::Stopwatch mStopwatch{false};
    };
}
