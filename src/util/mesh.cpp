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

    shader::MeshData Mesh::getMeshData(NormalTreatment normalTreatment) const {
        shader::MeshDataBuilder builder;

        for (size_t i = 0; i < mVertexData.size(); ++i) {
            auto const &vertexData = mVertexData[i];
            auto &outputVertex = builder.mVertices.emplace_back();
            outputVertex.mPosition = vertexData.mPosition;
            outputVertex.mUv = vertexData.mUv;
            outputVertex.mNormal = normalizeMaybeZero(vertexData.mNormal);
            outputVertex.mColor = vertexData.mColor;
            builder.addIndex(boost::safe_numerics::checked::cast<uint32_t>(i));
        }

        if (normalTreatment == NormalTreatment::AVERAGE) {
            averageNormals(builder);
        }
        return builder.build();
    }
} // namespace gfx_testing::util
