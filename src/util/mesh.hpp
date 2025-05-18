#pragma once
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <shader/shader_models.hpp>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <util/util.hpp>

namespace gfx_testing::util {

    enum class NormalTreatment {
        /**
         * Keeps the same number of vertices and averages the normals for each face together.
         * Not suitable for texture mapping.
         */
        AVERAGE,
        /**
         * Creates new vertices as needed to hold normals/UVs for all faces
         */
        SPLIT,
    };

    struct Triangle {
        std::array<size_t, 3> mVertexIndices;
    };

    class Mesh {
        struct Vertex {
            glm::vec3 mPosition{0};
            glm::vec2 mUv{0};
            glm::vec3 mNormal{0};
            glm::vec4 mColor{0};
        };

    public:
        NO_COPY(Mesh);
        Mesh() = default;

        Triangle const &addTriangle(glm::vec3 const &v1, glm::vec3 const &v2, glm::vec3 const &v3);

        void setColor(size_t index, glm::vec4 color);
        void setUv(size_t index, glm::vec2 uv);
        void setVertexNormal(size_t index, glm::vec3 normal);

        std::vector<size_t> getIndicesForPosition(glm::vec3 const &pos) const;

        shader::MeshData getMeshData(NormalTreatment normalTreatment) const;

    private:
        void averageNormals(shader::MeshDataBuilder &builder) const;


        std::vector<Vertex> mVertexData;
        std::vector<Triangle> mTriangles;
        std::unordered_map<glm::vec3, std::vector<size_t>> mVerticesByPosition;
    };
} // namespace gfx_testing::util
