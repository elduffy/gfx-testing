#pragma once
#include <optional>
#include <util/ref.hpp>


namespace gfx_testing::util {

    /**
     * Poor man's C++23 std::optional::transform
     */
    template<typename T, typename F, typename R = std::invoke_result_t<F, T>>
        requires std::invocable<F, T>
    std::optional<R> transform(std::optional<T> const &opt, F &&f) {
        if (opt.has_value()) {
            return f(opt.value());
        }
        return std::nullopt;
    }

    template<typename T>
    std::string toString(std::optional<T> const &opt, char const *defaultValue) {
        if (opt.has_value()) {
            std::stringstream ss;
            ss << opt.value();
            return ss.str();
        }
        return defaultValue;
    }

    template<typename T, typename F>
        requires std::invocable<F, T>
    void if_present(std::optional<T> const &opt, F &&f) {
        if (opt.has_value()) {
            f(opt.value());
        }
    }

    template<typename T>
    T const *nullable_pointer(std::optional<T> const &opt) {
        if (opt.has_value()) {
            return &opt.value();
        }
        return nullptr;
    }
} // namespace gfx_testing::util
