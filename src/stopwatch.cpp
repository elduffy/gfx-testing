#include <stopwatch.hpp>

namespace gfx_testing::util {
    Stopwatch::Stopwatch(bool start) {
        if (start) {
            mResumedTime = std::chrono::high_resolution_clock::now();
        }
    }

    uint64_t Stopwatch::getTime() const {
        if (isPaused()) {
            return mAccumulatedTime;
        }
        return mAccumulatedTime + std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::high_resolution_clock::now() - mResumedTime.value()).count();
    }

    void Stopwatch::toggle() {
        if (isPaused()) {
            resume();
        } else {
            pause();
        }
    }

    void Stopwatch::pause() {
        mAccumulatedTime = getTime();
        mResumedTime.reset();
    }

    void Stopwatch::resume() {
        mResumedTime = std::chrono::high_resolution_clock::now();
    }

}
