#pragma once
#include <chrono>
#include <optional>

#include <util/util.hpp>

namespace gfx_testing::util {
    class FpsCapper {
    public:
        NO_COPY_NO_MOVE(FpsCapper);
        explicit FpsCapper(std::optional<double> targetFps);

        void wait();

    private:
        std::optional<std::chrono::duration<double>> const mTargetLatency;
        std::chrono::duration<double> mMaxBusyWaitTime;
        std::chrono::time_point<std::chrono::steady_clock> mLastFrameTime;
    };
} // namespace gfx_testing::util
