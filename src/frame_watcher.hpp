#pragma once

#include <chrono>

namespace gfx_testing::util {
    class FrameWatcher {
        static uint64_t get_millis() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
        }

    public:
        void onFrame();

    private:
        uint64_t const mLogIntervalMs = 10000;
        uint64_t mSampleCount = 0;
        uint64_t mWindowStart = get_millis();
    };
}
