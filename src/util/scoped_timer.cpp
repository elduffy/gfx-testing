#include <util/scoped_timer.hpp>

namespace gfx_testing::util {
    ScopedTimer::ScopedTimer(std::chrono::high_resolution_clock::duration &result) :
        mResult(result), mStart(std::chrono::high_resolution_clock::now()) {
        mResult = std::chrono::high_resolution_clock::duration::zero();
    }

    ScopedTimer::~ScopedTimer() { mResult = std::chrono::high_resolution_clock::now() - mStart; }
} // namespace gfx_testing::util
