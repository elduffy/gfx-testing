#include <numeric>
#include <set>
#include <unordered_set>
#include <util/mesh.hpp>


namespace gfx_testing::util {

    glm::vec3 normalizeMaybeZero(glm::vec3 const &vec) {
        if (vec == glm::vec3(0)) {
            return vec;
        }
        return normalize(vec);
    }

    Triangle const &Mesh::addTriangle(glm::vec3 const &v1, glm::vec3 const &v2, glm::vec3 const &v3) {
        const auto idx1 = mVertexData.size();
        mVertexData.push_back({.mPosition = v1});
        mVertexData.push_back({.mPosition = v2});
        mVertexData.push_back({.mPosition = v3});

        mVerticesByPosition[v1].push_back(idx1);
        mVerticesByPosition[v2].push_back(idx1 + 1);
        mVerticesByPosition[v3].push_back(idx1 + 2);
        return mTriangles.emplace_back(std::array{idx1, idx1 + 1, idx1 + 2});
    }

    void Mesh::setColor(size_t index, glm::vec4 color) {
        CHECK_LT(index, mVertexData.size());
        mVertexData[index].mColor = std::move(color);
    }

    void Mesh::setUv(size_t index, glm::vec2 uv) {
        CHECK_LT(index, mVertexData.size());
        mVertexData[index].mUv = std::move(uv);
    }

    void Mesh::setVertexNormal(size_t index, glm::vec3 normal) {
        CHECK_LT(index, mVertexData.size());
        mVertexData[index].mNormal = std::move(normal);
    }

    void Mesh::averageNormals(shader::MeshDataBuilder &builder) const {
        for (const auto &indices: mVerticesByPosition | std::views::values) {
            CHECK(!indices.empty());

            glm::vec3 resultNormal{0};
            for (auto const idx: indices) {
                resultNormal += normalizeMaybeZero(mVertexData.at(idx).mNormal);
            }
            resultNormal /= static_cast<float>(indices.size());
            if (resultNormal != glm::vec3(0)) {
                resultNormal = normalizeMaybeZero(resultNormal);
            }
            for (auto const idx: indices) {
                builder.mVertices.at(idx).mNormal = resultNormal;
            }
        }
    }

    std::vector<size_t> Mesh::getIndicesForPosition(glm::vec3 const &pos) const {
        auto const iter = mVerticesByPosition.find(pos);
        if (iter == mVerticesByPosition.end()) {
            return {};
        }
        return iter->second;
    }

    shader::MeshData Mesh::getMeshData(AttribTreatment attribTreatment) const {

        const VertexOps vertexOps(attribTreatment);
        std::unordered_map<Vertex, size_t, VertexOps, VertexOps> vertexToUniqueIndex(1, vertexOps, vertexOps);
        shader::MeshDataBuilder builder;

        size_t uniqueIndex = 0;
        for (size_t i = 0; i < mVertexData.size(); ++i) {
            auto const &v = mVertexData[i];
            if (!vertexToUniqueIndex.contains(v)) {
                vertexToUniqueIndex[v] = uniqueIndex++;
                auto &outputVertex = builder.mVertices.emplace_back();
                outputVertex.mPosition = v.mPosition;
                if (attribTreatment.mTexCoord == TexCoordTreatment::DISCARD) {
                    outputVertex.mUv = {0, 0};
                } else {
                    outputVertex.mUv = v.mUv;
                }
                outputVertex.mNormal = normalizeMaybeZero(v.mNormal);
                outputVertex.mColor = v.mColor;
            }
        }

        for (size_t i = 0; i < mVertexData.size(); ++i) {
            auto const &v = mVertexData[i];
            auto const outIndex = vertexToUniqueIndex.at(v);
            builder.addIndex(util::narrow_u32(outIndex));
        }

        if (attribTreatment.mNormal == NormalTreatment::AVERAGE) {
            // averageNormals(builder);
            for (size_t outIdx = 0; outIdx < builder.mVertices.size(); ++outIdx) {
                auto const &outVertex = builder.mVertices[outIdx];

                std::unordered_set<glm::vec3> distinctNormals;
                for (auto const &inIdx: mVerticesByPosition.at(outVertex.mPosition)) {
                    distinctNormals.insert(normalizeMaybeZero(mVertexData.at(inIdx).mNormal));
                }
                glm::vec3 resultNormal = std::accumulate(distinctNormals.begin(), distinctNormals.end(), glm::vec3(0));
                resultNormal /= static_cast<float>(distinctNormals.size());
                resultNormal = normalizeMaybeZero(resultNormal);
                builder.mVertices.at(outIdx).mNormal = resultNormal;
            }
        }
        return builder.build();
    }
} // namespace gfx_testing::util
