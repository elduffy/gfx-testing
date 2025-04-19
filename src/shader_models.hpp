#pragma once
#include <vector>
#include <boost/safe_numerics/checked_default.hpp>
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

        [[nodiscard]] uint32_t getVertexBufferSize() const {
            return boost::safe_numerics::checked::cast<uint32_t>(vertices.size() * sizeof(PositionColorVertex));
        }

        [[nodiscard]] uint32_t getIndexBufferSize() const {
            return boost::safe_numerics::checked::cast<uint32_t>(indices.size() * sizeof(uint16_t));
        }
    };
}
