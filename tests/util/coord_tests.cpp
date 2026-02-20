#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/constants.hpp>
#include <glm/gtx/io.hpp>
#include <util/util.hpp>

#include "../matchers.hpp"
#undef CHECK
#undef FAIL
#include <catch2/catch_all.hpp>

using namespace gfx_testing::util;
using namespace gfx_testing::test;

constexpr float EPSILON = 0.0001f;

// --- getCartesianCoords ---

TEST_CASE("getCartesianCoords known values") {
    SECTION("x-axis: (r=5, theta=pi/2, phi=0)") {
        CHECK_THAT(getCartesianCoords({5, glm::half_pi<float>(), 0}), WithinAbs(glm::vec3{5, 0, 0}, EPSILON));
    }

    SECTION("y-axis: (r=5, theta=pi/2, phi=pi/2)") {
        CHECK_THAT(getCartesianCoords({5, glm::half_pi<float>(), glm::half_pi<float>()}),
                   WithinAbs(glm::vec3{0, 5, 0}, EPSILON));
    }

    SECTION("+z-axis: (r=3, theta=0)") {
        CHECK_THAT(getCartesianCoords({3, 0, 0}), WithinAbs(glm::vec3{0, 0, 3}, EPSILON));
    }

    SECTION("-z-axis: (r=3, theta=pi)") {
        CHECK_THAT(getCartesianCoords({3, glm::pi<float>(), 0}), WithinAbs(glm::vec3{0, 0, -3}, EPSILON));
    }

    SECTION("diagonal: (r=sqrt(3), theta=acos(1/sqrt(3)), phi=pi/4) -> (1,1,1)") {
        float const r = glm::root_three<float>();
        float const t = std::acos(1.f / glm::root_three<float>());
        float const p = glm::quarter_pi<float>();
        CHECK_THAT(getCartesianCoords({r, t, p}), WithinAbs(glm::vec3{1, 1, 1}, EPSILON));
    }
}

// --- getSphericalCoords ---

TEST_CASE("getSphericalCoords known values") {
    SECTION("x-axis: (5,0,0) -> (r=5, theta=pi/2, phi=0)") {
        auto const s = getSphericalCoords({5, 0, 0});
        CHECK_THAT(s.x, Catch::Matchers::WithinAbs(5.f, EPSILON));
        CHECK_THAT(s.y, Catch::Matchers::WithinAbs(glm::half_pi<float>(), EPSILON));
        CHECK_THAT(s.z, Catch::Matchers::WithinAbs(0.f, EPSILON));
    }

    SECTION("y-axis: (0,5,0) -> (r=5, theta=pi/2, phi=pi/2)") {
        auto const s = getSphericalCoords({0, 5, 0});
        CHECK_THAT(s.x, Catch::Matchers::WithinAbs(5.f, EPSILON));
        CHECK_THAT(s.y, Catch::Matchers::WithinAbs(glm::half_pi<float>(), EPSILON));
        CHECK_THAT(s.z, Catch::Matchers::WithinAbs(glm::half_pi<float>(), EPSILON));
    }

    SECTION("radius is sqrt(x^2 + y^2 + z^2)") {
        CHECK_THAT(getSphericalCoords({3, 4, 0}).x, Catch::Matchers::WithinAbs(5.f, EPSILON));
        CHECK_THAT(getSphericalCoords({0, 0, 7}).x, Catch::Matchers::WithinAbs(7.f, EPSILON));
    }
}

// --- round trips ---

TEST_CASE("cartesian -> spherical -> cartesian round trip") {
    auto const roundTrip = [](glm::vec3 const &v) { return getCartesianCoords(getSphericalCoords(v)); };

    SECTION("x-axis") { CHECK_THAT(roundTrip({5, 0, 0}), WithinAbs(glm::vec3{5, 0, 0}, EPSILON)); }

    SECTION("y-axis") { CHECK_THAT(roundTrip({0, 3, 0}), WithinAbs(glm::vec3{0, 3, 0}, EPSILON)); }

    SECTION("arbitrary points") {
        CHECK_THAT(roundTrip({1, 2, 3}), WithinAbs(glm::vec3{1, 2, 3}, EPSILON));
        CHECK_THAT(roundTrip({-1, 3, -2}), WithinAbs(glm::vec3{-1, 3, -2}, EPSILON));
        CHECK_THAT(roundTrip({0, -4, 5}), WithinAbs(glm::vec3{0, -4, 5}, EPSILON));
    }
}

TEST_CASE("spherical -> cartesian -> spherical round trip") {
    auto const roundTrip = [](glm::vec3 const &s) { return getSphericalCoords(getCartesianCoords(s)); };

    SECTION("equatorial point") {
        auto const s = roundTrip({5, glm::half_pi<float>(), 0});
        CHECK_THAT(s.x, Catch::Matchers::WithinAbs(5.f, EPSILON));
        CHECK_THAT(s.y, Catch::Matchers::WithinAbs(glm::half_pi<float>(), EPSILON));
    }

    SECTION("off-equator point") {
        auto const s = roundTrip({4, glm::quarter_pi<float>(), glm::half_pi<float>()});
        CHECK_THAT(s.x, Catch::Matchers::WithinAbs(4.f, EPSILON));
        CHECK_THAT(s.y, Catch::Matchers::WithinAbs(glm::quarter_pi<float>(), EPSILON));
        CHECK_THAT(s.z, Catch::Matchers::WithinAbs(glm::half_pi<float>(), EPSILON));
    }
}
