#pragma once
#include <vector>
#include <boost/safe_numerics/checked_default.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#define SHADER_ALIGN alignas(16)

namespace gfx_testing::shader {
    struct PositionColorVertex {
        glm::vec3 mPosition;
        glm::vec3 mNormal;
        glm::vec4 mColor;
    };

    struct MVPMatrix {
        glm::mat4 mTransform;
    };

    struct MeshData {
        std::vector<PositionColorVertex> mVertices;
        std::vector<uint16_t> mIndices;

        [[nodiscard]] uint32_t getVertexBufferSize() const {
            return boost::safe_numerics::checked::cast<uint32_t>(mVertices.size() * sizeof(PositionColorVertex));
        }

        [[nodiscard]] uint32_t getIndexBufferSize() const {
            return boost::safe_numerics::checked::cast<uint32_t>(mIndices.size() * sizeof(uint16_t));
        }
    };

    struct GoochParams {
        SHADER_ALIGN glm::vec3 mViewDir;
        SHADER_ALIGN glm::vec3 mLightDir;
        SHADER_ALIGN glm::vec3 mCoolColor;
        SHADER_ALIGN glm::vec3 mWarmColor;
    };
}
