#pragma once
#include <array>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <shader/shader_models.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <shader/mesh.hpp>
#include <util/util.hpp>

namespace gfx_testing::util {

    enum class NormalTreatment {
        /**
         * Each vertex retains its normals.
         */
        SPLIT,
        /**
         * For each distinct vertex position, averages the corresponding normals.
         */
        AVERAGE,
    };

    enum class TexCoordTreatment {
        /**
         * Each vertex retains its texture coordinates.
         */
        SPLIT,
        /**
         * Discard texture coordinates. In the resulting MeshData they will be (0,0).
         * Allows for fewer vertices in some cases.
         */
        DISCARD,
    };

    struct AttribTreatment {
        NormalTreatment mNormal{NormalTreatment::SPLIT};
        TexCoordTreatment mTexCoord{TexCoordTreatment::SPLIT};
    };

    struct Triangle {
        std::array<size_t, 3> mVertexIndices;
    };

    class Mesh {
        struct Vertex {
            glm::vec3 mPosition{0};
            glm::vec2 mUv{0};
            glm::vec3 mNormal{0};
            glm::vec4 mColor{1};
        };

        struct VertexOps {
            explicit VertexOps(AttribTreatment attribTreatment) : mAttribTreatment(attribTreatment) {}

            size_t operator()(const Vertex &obj) const {
                std::size_t seed = 0x31B8814C;
                seed ^= (seed << 6) + (seed >> 2) + 0x732CB696 + std::hash<glm::vec3>()(obj.mPosition);
                if (mAttribTreatment.mTexCoord != TexCoordTreatment::DISCARD) {
                    seed ^= (seed << 6) + (seed >> 2) + 0x1FF41DF2 + std::hash<glm::vec2>()(obj.mUv);
                }
                if (mAttribTreatment.mNormal != NormalTreatment::AVERAGE) {
                    seed ^= (seed << 6) + (seed >> 2) + 0x03303582 + std::hash<glm::vec3>()(obj.mNormal);
                }
                seed ^= (seed << 6) + (seed >> 2) + 0x47695477 + std::hash<glm::vec4>()(obj.mColor);
                return seed;
            }

            bool operator()(const Vertex &lhs, const Vertex &rhs) const {
                return lhs.mPosition == rhs.mPosition && lhs.mColor == rhs.mColor &&
                       (mAttribTreatment.mTexCoord == TexCoordTreatment::DISCARD || lhs.mUv == rhs.mUv) &&
                       (mAttribTreatment.mNormal == NormalTreatment::AVERAGE || lhs.mNormal == rhs.mNormal);
            }

            AttribTreatment mAttribTreatment;
        };

    public:
        NO_COPY(Mesh);
        Mesh() = default;

        Triangle const &addTriangle(glm::vec3 const &v1, glm::vec3 const &v2, glm::vec3 const &v3);

        void setColor(size_t index, glm::vec4 color);
        void setUv(size_t index, glm::vec2 uv);
        void setVertexNormal(size_t index, glm::vec3 normal);

        std::vector<size_t> getIndicesForPosition(glm::vec3 const &pos) const;

        shader::MeshData getMeshData(AttribTreatment attribTreatment) const;

    private:
        void averageNormals(shader::MeshDataBuilder &builder) const;

        std::vector<Vertex> mVertexData;
        std::vector<Triangle> mTriangles;
        std::unordered_map<glm::vec3, std::vector<size_t>> mVerticesByPosition;
    };
} // namespace gfx_testing::util
