#include <frame_watcher.hpp>
#include <SDL3/SDL_log.h>

namespace gfx_testing::util {
    void FrameWatcher::onFrame() {
        auto const now = get_millis();
        auto const windowLength = now - mWindowStart;
        if (windowLength > mLogIntervalMs) {
            auto const rate = (mSampleCount * 1000.0) / windowLength;
            SDL_Log("Frame rate: %f", rate);
            mWindowStart = now;
            mSampleCount = 0;
        }
        ++mSampleCount;
    }
}
