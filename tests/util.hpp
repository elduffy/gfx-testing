#pragma once
#include <glm/vec3.hpp>
#include <shader/shader_models.hpp>
#include <unordered_set>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <shader/mesh.hpp>

namespace gfx_testing::test {
    std::unordered_set<glm::vec3> getUniqueVertexPositions(shader::MeshData const &meshData) {
        std::unordered_set<glm::vec3> result;
        for (auto const &v: meshData.mVertices) {
            result.insert(v.mPosition);
        }
        return result;
    }
} // namespace gfx_testing::test
