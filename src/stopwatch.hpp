#pragma once
#include <chrono>
#include <cstdint>

namespace gfx_testing::util {
    class Stopwatch {
    public:
        explicit Stopwatch(bool start);

        [[nodiscard]] uint64_t getTime() const;

        void toggle();

        void pause();

        void resume();

        [[nodiscard]] bool isPaused() const {
            return !mResumedTime.has_value();
        }

    private:
        uint64_t mAccumulatedTime = 0;
        std::optional<std::chrono::high_resolution_clock::time_point> mResumedTime = std::nullopt;
    };

    struct Snapshot {
        uint64_t mAccumulatedTime;
        float mDeltaTime;

        void update(Stopwatch const &stopwatch) {
            auto const lastTime = mAccumulatedTime;
            mAccumulatedTime = stopwatch.getTime();
            mDeltaTime = static_cast<float>(mAccumulatedTime - lastTime) / 1000.f;
        }
    };
}
