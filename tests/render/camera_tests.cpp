#define GLM_ENABLE_EXPERIMENTAL
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/io.hpp>
#include <glm/vec4.hpp>
#include <render/camera.hpp>

#include "../matchers.hpp"
#undef CHECK
#undef FAIL
#include <catch2/catch_all.hpp>

using namespace gfx_testing::render;
using namespace gfx_testing::test;

constexpr float EPSILON = 0.0001f;

TEST_CASE("Camera initial state") {
    Camera cam({5, 0, 0});
    CHECK_THAT(cam.getPosition(), WithinAbs(glm::vec3{5, 0, 0}, EPSILON));
    CHECK_THAT(cam.getPivot(), WithinAbs(glm::vec3{0, 0, 0}, EPSILON));
}

TEST_CASE("Camera setPosition and setPivot") {
    Camera cam({5, 0, 0});
    cam.setPosition({1, 2, 3});
    cam.setPivot({4, 5, 6});
    CHECK_THAT(cam.getPosition(), WithinAbs(glm::vec3{1, 2, 3}, EPSILON));
    CHECK_THAT(cam.getPivot(), WithinAbs(glm::vec3{4, 5, 6}, EPSILON));
}

TEST_CASE("Camera approach") {
    // Camera on x-axis at distance 5, looking at origin.
    // Approach changes radius in spherical coords; pivot stays fixed.
    Camera cam({5, 0, 0});

    SECTION("negative delta moves closer") {
        cam.approach(-2);
        CHECK_THAT(cam.getPosition(), WithinAbs(glm::vec3{3, 0, 0}, EPSILON));
    }

    SECTION("positive delta moves farther") {
        cam.approach(2);
        CHECK_THAT(cam.getPosition(), WithinAbs(glm::vec3{7, 0, 0}, EPSILON));
    }

    SECTION("clamped to MIN_RADIUS of 1") {
        cam.approach(-100);
        CHECK_THAT(cam.getPosition(), WithinAbs(glm::vec3{1, 0, 0}, EPSILON));
    }

    SECTION("pivot is unchanged") {
        cam.approach(-2);
        CHECK_THAT(cam.getPivot(), WithinAbs(glm::vec3{0, 0, 0}, EPSILON));
    }
}

TEST_CASE("Camera pivot") {
    // Camera starts on x-axis at distance 5: spherical (r=5, theta=pi/2, phi=0).
    Camera cam({5, 0, 0});

    SECTION("decreasing theta lifts camera toward +z") {
        // theta: pi/2 -> pi/4; new cartesian: (5/sqrt(2), 0, 5/sqrt(2))
        cam.pivot({-glm::quarter_pi<float>(), 0});
        float const expected = 5.f / glm::root_two<float>();
        CHECK_THAT(cam.getPosition(), WithinAbs(glm::vec3{expected, 0, expected}, EPSILON));
    }

    SECTION("increasing phi rotates camera toward +y") {
        // phi: 0 -> pi/2; new cartesian: (0, 5, 0)
        cam.pivot({0, glm::half_pi<float>()});
        CHECK_THAT(cam.getPosition(), WithinAbs(glm::vec3{0, 5, 0}, EPSILON));
    }

    SECTION("pivot point is unchanged") {
        cam.pivot({0.3f, 0.7f});
        CHECK_THAT(cam.getPivot(), WithinAbs(glm::vec3{0, 0, 0}, EPSILON));
    }

    SECTION("radius is preserved after pivot") {
        cam.pivot({0.5f, 1.2f});
        CHECK_THAT(glm::length(cam.getPosition() - cam.getPivot()), Catch::Matchers::WithinAbs(5.f, EPSILON));
    }

    SECTION("theta is clamped above zero (camera stays below +z pole)") {
        cam.pivot({-100, 0});
        CHECK_THAT(glm::length(cam.getPosition()), Catch::Matchers::WithinAbs(5.f, EPSILON));
        CHECK(cam.getPosition().z > 0);
    }

    SECTION("theta is clamped below pi (camera stays above -z pole)") {
        cam.pivot({100, 0});
        CHECK_THAT(glm::length(cam.getPosition()), Catch::Matchers::WithinAbs(5.f, EPSILON));
        CHECK(cam.getPosition().z < 0);
    }
}

TEST_CASE("Camera translate") {
    // Camera on x-axis at distance 5. In this orientation:
    //   forward = {-1, 0, 0}, right = {0, 1, 0}, up = {0, 0, 1}
    Camera cam({5, 0, 0});

    SECTION("translate along right axis shifts both position and pivot") {
        cam.translate({1, 0});
        CHECK_THAT(cam.getPosition(), WithinAbs(glm::vec3{5, 1, 0}, EPSILON));
        CHECK_THAT(cam.getPivot(), WithinAbs(glm::vec3{0, 1, 0}, EPSILON));
    }

    SECTION("translate along up axis shifts both position and pivot") {
        cam.translate({0, 1});
        CHECK_THAT(cam.getPosition(), WithinAbs(glm::vec3{5, 0, 1}, EPSILON));
        CHECK_THAT(cam.getPivot(), WithinAbs(glm::vec3{0, 0, 1}, EPSILON));
    }

    SECTION("camera-to-pivot vector is unchanged by translate") {
        glm::vec3 const initialDelta = cam.getPivot() - cam.getPosition();
        cam.translate({3, -2});
        glm::vec3 const newDelta = cam.getPivot() - cam.getPosition();
        CHECK_THAT(newDelta, WithinAbs(initialDelta, EPSILON));
    }
}

TEST_CASE("Camera view matrix") {
    // Camera on x-axis at distance 5, pivot at origin.
    Camera cam({5, 0, 0});
    auto const view = cam.computeViewMatrix();

    SECTION("camera position maps to view-space origin") {
        auto const posInView = view * glm::vec4(cam.getPosition(), 1);
        CHECK_THAT(glm::vec3{posInView}, WithinAbs(glm::vec3{0, 0, 0}, EPSILON));
    }

    SECTION("pivot lies on the negative z-axis in view space") {
        auto const pivotInView = view * glm::vec4(cam.getPivot(), 1);
        CHECK(pivotInView.z < 0);
    }
}
