#pragma once

#include <pipeline/pipelines.hpp>
#include <render/samplers.hpp>
#include <sdl.hpp>
#include <util/fps_capper.hpp>
#include <util/stopwatch.hpp>

namespace gfx_testing::game {
    struct GameSettings {
        std::optional<float> mTargetFps = std::nullopt;
    };

    class GameContext {
    public:
        GameContext(sdl::SdlContext const &sdlContext, util::ResourceLoader const &resourceLoader,
                    GameSettings const &settings);

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

        [[nodiscard]] util::Snapshot const &getFrameSnapshot() const { return mLastFrame; }

        void maybeLimitFps() { mFpsCapper.wait(); }

    private:
        util::Snapshot mLastFrame{};

    public:
        sdl::SdlContext const &mSdlContext;
        util::ResourceLoader const &mResourceLoader;
        pipeline::Pipelines mPipelines;
        render::Samplers mSamplers;
        util::Stopwatch mStopwatch{false};
        util::FpsCapper mFpsCapper;
    };
} // namespace gfx_testing::game
