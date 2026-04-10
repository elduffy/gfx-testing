#include <thread>

#include <SDL3/SDL_log.h>
#include <util/fps_capper.hpp>

namespace gfx_testing::util {
    // Max percentage of the frame time spent busy waiting, rather than sleeping.
    // Larger values means more CPU use, but a more accurate FPS.
    constexpr double BUSY_WAIT_PERCENT = 0.02;

    FpsCapper::FpsCapper(std::optional<double> targetFps) :
        mTargetLatency(targetFps.has_value() ? std::optional(1.0 / targetFps.value()) : std::nullopt),
        mMaxBusyWaitTime(mTargetLatency.has_value() ? mTargetLatency.value() * BUSY_WAIT_PERCENT
                                                    : std::chrono::microseconds::zero()),
        mLastFrameTime(std::chrono::steady_clock::now()) {}

    void FpsCapper::wait() {
        if (!mTargetLatency.has_value()) {
            return;
        }
        const auto now = std::chrono::steady_clock::now();
        const auto actualDuration = now - mLastFrameTime;
        const auto timeToWait = mTargetLatency.value() - actualDuration;
        auto const exitTime = now + timeToWait;
        std::this_thread::sleep_for(timeToWait - mMaxBusyWaitTime);
        while ((mLastFrameTime = std::chrono::steady_clock::now()) < exitTime) {
            // busy wait
        }
    }
} // namespace gfx_testing::util
