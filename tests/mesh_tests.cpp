#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>
#include <util/mesh.hpp>

#include "matchers.hpp"
#undef CHECK // Use the Catch2 CHECK in this file
#include <catch2/catch_all.hpp>
#include <iostream>

using namespace gfx_testing::util;
using namespace gfx_testing::shader;
using namespace gfx_testing::test;

constexpr glm::vec3 V1{0, 0, 0};
constexpr glm::vec4 C1{1, 0, 0, 1};
constexpr glm::vec3 V2{1, 0, 0};
constexpr glm::vec4 C2{0, 1, 0, 1};
constexpr glm::vec3 V3{1, 1, 0};
constexpr glm::vec4 C3{0, 0, 1, 1};
constexpr glm::vec3 V4{0, 1, 0};
constexpr glm::vec4 C4{1, 0, 1, 1};

VertexData makeVertexData(glm::vec3 pos, glm::vec2 uv, glm::vec3 normal, glm::vec4 color) {
    return {
            .mPosition = pos,
            .mUv = uv,
            .mNormal = normal,
            .mColor = color,
    };
}

constexpr float EPSILON = 0.0001f;


TEST_CASE("Build a triangle") {
    Mesh mesh;

    auto const &t1 = mesh.addTriangle(V1, V2, V3);
    CHECK(t1.mVertexIndices[0] == 0);
    CHECK(t1.mVertexIndices[1] == 1);
    CHECK(t1.mVertexIndices[2] == 2);
    mesh.setColor(0, C1);
    mesh.setColor(1, C2);
    mesh.setColor(2, C3);
    for (auto const normalTreatment: {NormalTreatment::AVERAGE, NormalTreatment::SPLIT}) {
        auto const meshData = mesh.getMeshData(normalTreatment);
        CHECK(meshData.mVertices.at(0) == makeVertexData(V1, {0, 0}, {0, 0, 0}, C1));
        CHECK(meshData.mVertices.at(1) == makeVertexData(V2, {0, 0}, {0, 0, 0}, C2));
        CHECK(meshData.mVertices.at(2) == makeVertexData(V3, {0, 0}, {0, 0, 0}, C3));
    }
    mesh.setUv(1, {0, 1});
    mesh.setVertexNormal(1, {1, 1, 1});
    mesh.setUv(2, {.5, .5});
    mesh.setVertexNormal(2, {.5, .5, .5});
    for (auto const normalTreatment: {NormalTreatment::AVERAGE, NormalTreatment::SPLIT}) {
        const auto N2 = glm::normalize(glm::vec3{1, 1, 1});
        const auto N3 = glm::normalize(glm::vec3{.5, .5, .5});
        auto const meshData = mesh.getMeshData(normalTreatment);
        CHECK_THAT(meshData.mVertices.at(0), WithinAbs(makeVertexData(V1, {0, 0}, {0, 0, 0}, C1), EPSILON));
        CHECK_THAT(meshData.mVertices.at(1), WithinAbs(makeVertexData(V2, {0, 1}, N2, C2), EPSILON));
        CHECK_THAT(meshData.mVertices.at(2), WithinAbs(makeVertexData(V3, {.5, .5}, N3, C3), EPSILON));
    }
}

TEST_CASE("Build a quad") {
    Mesh mesh;
    auto const &t1 = mesh.addTriangle(V1, V2, V3);
    CHECK(t1.mVertexIndices[0] == 0);
    CHECK(t1.mVertexIndices[1] == 1);
    CHECK(t1.mVertexIndices[2] == 2);
    mesh.setColor(0, C1);
    mesh.setColor(1, C2);
    mesh.setColor(2, C3);

    CHECK_THAT(mesh.getIndicesForPosition(V1), Catch::Matchers::UnorderedEquals(std::vector<size_t>{0}));
    CHECK_THAT(mesh.getIndicesForPosition(V2), Catch::Matchers::UnorderedEquals(std::vector<size_t>{1}));
    CHECK_THAT(mesh.getIndicesForPosition(V3), Catch::Matchers::UnorderedEquals(std::vector<size_t>{2}));

    auto const &t2 = mesh.addTriangle(V1, V3, V4);
    CHECK(t2.mVertexIndices[0] == 3);
    CHECK(t2.mVertexIndices[1] == 4);
    CHECK(t2.mVertexIndices[2] == 5);
    mesh.setColor(3, C1);
    mesh.setColor(4, C3);
    mesh.setColor(5, C4);

    CHECK_THAT(mesh.getIndicesForPosition(V1), Catch::Matchers::UnorderedEquals(std::vector<size_t>{0, 3}));
    CHECK_THAT(mesh.getIndicesForPosition(V2), Catch::Matchers::UnorderedEquals(std::vector<size_t>{1}));
    CHECK_THAT(mesh.getIndicesForPosition(V3), Catch::Matchers::UnorderedEquals(std::vector<size_t>{2, 4}));
    CHECK_THAT(mesh.getIndicesForPosition(V4), Catch::Matchers::UnorderedEquals(std::vector<size_t>{5}));

    for (auto const normalTreatment: {NormalTreatment::AVERAGE, NormalTreatment::SPLIT}) {
        auto const meshData = mesh.getMeshData(normalTreatment);
        REQUIRE(meshData.mVertices.size() == 6);
        for (auto const &v: meshData.mVertices) {
            CHECK(v.mUv == glm::vec2(0));
            CHECK(v.mNormal == glm::vec3(0));
        }
    }
    // v0 aliases
    mesh.setVertexNormal(0, {1, 0, 0});
    mesh.setVertexNormal(3, {0, 1, 0});
    // v2 aliases
    mesh.setVertexNormal(2, {1, 2, 0});
    mesh.setVertexNormal(4, {3, 4, 0});

    {
        auto const meshData = mesh.getMeshData(NormalTreatment::AVERAGE);
        REQUIRE(meshData.mVertices.size() == 6);
        CHECK_THAT(meshData.mVertices[0].mNormal, WithinAbs(glm::normalize(glm::vec3{0.5, 0.5, 0}), EPSILON));
        CHECK_THAT(meshData.mVertices[3].mNormal, WithinAbs(glm::normalize(glm::vec3{0.5, 0.5, 0}), EPSILON));

        auto const expectedNorm =
                glm::normalize((glm::normalize(glm::vec3{1, 2, 0}) + glm::normalize(glm::vec3{3, 4, 0})) * 0.5f);
        CHECK_THAT(meshData.mVertices[2].mNormal, WithinAbs(expectedNorm, EPSILON));
        CHECK_THAT(meshData.mVertices[4].mNormal, WithinAbs(expectedNorm, EPSILON));

        CHECK(meshData.mIndices.asVector<uint16_t>() == std::vector<uint16_t>{0, 1, 2, 3, 4, 5});
        CHECK(meshData.mVertices[0].mColor == C1);
        CHECK(meshData.mVertices[0].mPosition == V1);
        CHECK(meshData.mVertices[1].mColor == C2);
        CHECK(meshData.mVertices[1].mPosition == V2);
        CHECK(meshData.mVertices[2].mColor == C3);
        CHECK(meshData.mVertices[2].mPosition == V3);
        CHECK(meshData.mVertices[3].mColor == C1);
        CHECK(meshData.mVertices[3].mPosition == V1);
        CHECK(meshData.mVertices[4].mColor == C3);
        CHECK(meshData.mVertices[4].mPosition == V3);
        CHECK(meshData.mVertices[5].mColor == C4);
        CHECK(meshData.mVertices[5].mPosition == V4);
    }

    {
        auto const meshData = mesh.getMeshData(NormalTreatment::SPLIT);
        REQUIRE(meshData.mVertices.size() == 6);
        CHECK(meshData.mVertices[0].mNormal == glm::vec3{1, 0, 0});
        CHECK(meshData.mVertices[3].mNormal == glm::vec3{0, 1, 0});

        CHECK_THAT(meshData.mVertices[2].mNormal, WithinAbs(glm::normalize(glm::vec3{1, 2, 0}), EPSILON));
        CHECK_THAT(meshData.mVertices[4].mNormal, WithinAbs(glm::normalize(glm::vec3{3, 4, 0}), EPSILON));

        CHECK(meshData.mIndices.asVector<uint16_t>() == std::vector<uint16_t>{0, 1, 2, 3, 4, 5});
        CHECK(meshData.mVertices[0].mColor == C1);
        CHECK(meshData.mVertices[0].mPosition == V1);
        CHECK(meshData.mVertices[1].mColor == C2);
        CHECK(meshData.mVertices[1].mPosition == V2);
        CHECK(meshData.mVertices[2].mColor == C3);
        CHECK(meshData.mVertices[2].mPosition == V3);
        CHECK(meshData.mVertices[3].mColor == C1);
        CHECK(meshData.mVertices[3].mPosition == V1);
        CHECK(meshData.mVertices[4].mColor == C3);
        CHECK(meshData.mVertices[4].mPosition == V3);
        CHECK(meshData.mVertices[5].mColor == C4);
        CHECK(meshData.mVertices[5].mPosition == V4);
    }
}
