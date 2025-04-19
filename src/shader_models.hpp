#pragma once
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace gfx_testing::shader {
    struct PositionColorVertex {
        glm::vec3 mPosition;
        glm::vec4 mColor;
    };

    struct MVPMatrix {
        glm::mat4 mTransform;
    };

    struct MeshData {
        std::vector<PositionColorVertex> vertices;
        std::vector<uint16_t> indices;
    };
}
