#include <render/camera.hpp>
#include <util.hpp>

#include "glm/trigonometric.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace gfx_testing::render {
    Camera::Camera(glm::vec3 const &initialPos):
        mPosWs(initialPos), mView(glm::identity<glm::mat4x4>()) {
        updateViewMatrix();
    }

    void Camera::pivot(glm::vec2 const &radians) {
        // Using ISO/physics convention.
        // radians.x (theta): pivot top to bottom
        // radians.y (phi): pivot left to right
        auto const deltaTheta = radians.x;
        auto const deltaPhi = radians.y;
        auto newSpherical = util::getSphericalCoords(mPosWs) + glm::vec3{0, deltaTheta, deltaPhi};
        constexpr auto MIN_THETA = glm::radians(.1f);
        constexpr auto MAX_THETA = glm::radians(180.f) - MIN_THETA;
        newSpherical.y = glm::clamp(newSpherical.y, MIN_THETA, MAX_THETA);
        mPosWs = util::getCartesianCoords(newSpherical);
        updateViewMatrix();
    }

    void Camera::approach(float const deltaRadius) {
        constexpr auto MIN_RADIUS = 1.f;
        auto newSpherical = util::getSphericalCoords(mPosWs) + glm::vec3(deltaRadius, 0, 0);
        newSpherical.x = std::max(newSpherical.x, MIN_RADIUS);
        mPosWs = util::getCartesianCoords(newSpherical);
        updateViewMatrix();
    }

    void Camera::updateViewMatrix() {
        mView = lookAt(mPosWs, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    }
}
