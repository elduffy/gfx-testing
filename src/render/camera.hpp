#pragma once

#include <glm/vec3.hpp>

#include "glm/mat4x4.hpp"

namespace gfx_testing::render {
    class Camera {
    public:
        explicit Camera(glm::vec3 const &initialPos);

        void pivot(glm::vec2 const &radians);

        void approach(float deltaRadius);

    private:
        void updateViewMatrix();

    public:
        glm::vec3 mPosWs;
        glm::mat4x4 mView;
    };
} // namespace gfx_testing::render
