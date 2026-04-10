#pragma once

#include <ostream>

#include <ecs/ecs.hpp>
#include <pipeline/pipelines.hpp>
#include <render/samplers.hpp>
#include <sdl.hpp>
#include <util/fps_capper.hpp>
#include <util/optional.hpp>
#include <util/stopwatch.hpp>

namespace gfx_testing::game {
    struct GameSettings {
        // Optional FPS to target.
        // std::nullopt causes vsync to be used.
        std::optional<float> mTargetFps = std::nullopt;
        bool mVsyncDisabled = false;
        bool mHdrEnabled = false;
        bool mHeadless = false;
        uint32_t mFrameCount = 1;

        [[nodiscard]] std::vector<SDL_GPUPresentMode> getPresentModes() const;
        [[nodiscard]] std::vector<SDL_GPUSwapchainComposition> getSwapchainCompositions() const;

        friend std::ostream &operator<<(std::ostream &os, const GameSettings &settings) {
            return os << "  mTargetFps: " << util::toString(settings.mTargetFps, "none") << "\n"
                      << "  mVsyncDisabled: " << util::toString(settings.mVsyncDisabled);
        }
    };

    class GameContext {
    public:
        GameContext(sdl::SdlContext const &sdlContext, io::ResourceLoader const &resourceLoader,
                    GameSettings const &settings);

        template<typename EventFn, typename UpdateFn>
        void runMainLoop(EventFn &eventFn, UpdateFn &updateFn, bool pollEvents = true,
                         std::optional<uint32_t> frameCount = std::nullopt) {
            SDL_Event event;
            uint32_t framesRendered = 0;

            mStopwatch.resume();
            while (!frameCount.has_value() || framesRendered < frameCount.value()) {
                mLastFrame.update(mStopwatch);

                if (pollEvents) {
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
                }
                updateFn();
                ++framesRendered;
            }
        }

        [[nodiscard]] util::Snapshot const &getFrameSnapshot() const { return mLastFrame; }

        void maybeLimitFps() { mFpsCapper.wait(); }

        ecs::Ecs &getEcs() { return mEcs; }

    private:
        util::Snapshot mLastFrame{};

    public:
        sdl::SdlContext const &mSdlContext;
        io::ResourceLoader const &mResourceLoader;
        pipeline::Pipelines mPipelines;
        render::Samplers mSamplers;
        util::Stopwatch mStopwatch{false};
        GameSettings const &mGameSettings;
        util::FpsCapper mFpsCapper;

    private:
        ecs::Ecs mEcs;
    };
} // namespace gfx_testing::game
