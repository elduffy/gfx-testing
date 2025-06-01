#pragma once

#include <glm/vec3.hpp>

#include "glm/mat4x4.hpp"

namespace gfx_testing::render {
    class Camera {
        static constexpr glm::vec3 UP{0, 0, 1};

    public:
        explicit Camera(glm::vec3 const &initialPos);

        void pivot(glm::vec2 const &radians);

        void approach(float deltaRadius);

        void translate(glm::vec2 const &planeDelta);

    private:
        void updateViewMatrix();

    public:
        glm::vec3 mPosWs;
        glm::vec3 mPivot{0, 0, 0};
        glm::mat4x4 mView;
    };
} // namespace gfx_testing::render
