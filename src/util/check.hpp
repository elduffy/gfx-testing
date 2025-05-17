#pragma once

#define BOOST_ENABLE_ASSERT_HANDLE
#define BOOST_ENABLE_ASSERT_DEBUG_HANDLER
#include <boost/assert.hpp>

#define CHECK1(expr) BOOST_ASSERT(expr)
#define CHECK2(expr, msg) BOOST_ASSERT_MSG(expr, msg)
#define GET_CHECK(_1, _2, NAME, ...) NAME
#define CHECK(...) GET_CHECK(__VA_ARGS__, CHECK2, CHECK1)(__VA_ARGS__)


inline void boost::assertion_failed(char const *expr, char const *function, char const *file, long line) {
    throw std::runtime_error(std::format("Assertion '{}' failed in '{}' at {}:{}", expr, function, file, line));
}

inline void boost::assertion_failed_msg(char const *expr, char const *msg, char const *function, char const *file,
                                        long line) {
    throw std::runtime_error(std::format("Assertion '{}' failed in '{}' at {}:{} {}", expr, function, file, line, msg));
}
