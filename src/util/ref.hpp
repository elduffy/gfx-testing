#pragma once
#include <functional>

namespace gfx_testing::util {
    template<typename T>
    using ref = std::reference_wrapper<T>;

    template<typename T>
    using cref = std::reference_wrapper<const T>;

    template<typename T>
    using ref_opt = std::optional<ref<T>>;

    template<typename T>
    using cref_opt = std::optional<cref<T>>;

    template<typename T>
    using ref_vec = std::vector<ref<T>>;

    template<typename T>
    using cref_vec = std::vector<cref<T>>;
} // namespace gfx_testing::util
