#pragma once

#include <SDL3/SDL_log.h>
#include <boost/scope/scope_exit.hpp>
#include <chrono>

namespace gfx_testing::util {
    class ScopedTimer {
    public:
        explicit ScopedTimer(std::chrono::high_resolution_clock::duration &result);
        ~ScopedTimer();

    private:
        std::chrono::high_resolution_clock::duration &mResult;
        std::chrono::high_resolution_clock::time_point mStart;
    };

    template<typename Fn, typename... Args>
        requires std::invocable<Fn, Args...>
    auto logDuration(std::string_view name, Fn &fn, Args &&...args) {
        std::chrono::high_resolution_clock::duration measurement;
        auto log = boost::scope::make_scope_exit([&measurement, name] {
            const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(measurement);
            SDL_Log("%s: %lu ms", name.data(), millis.count());
        });
        {
            ScopedTimer timer(measurement);
            return fn(std::forward<Args>(args)...);
        }
    }
} // namespace gfx_testing::util

#define LOG_DURATION(Fn, ...) ::gfx_testing::util::logDuration(#Fn, Fn, __VA_ARGS__);
