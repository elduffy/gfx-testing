#pragma once

namespace gfx_testing::util {
    static constexpr bool DEBUG_MODE =
#ifdef NDEBUG
            false;
#else
            true;
#endif

} // namespace gfx_testing::util
