
#include <util/gltf_loader.hpp>
#undef CHECK // Use the Catch2 CHECK in this file
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "util.hpp"

using namespace gfx_testing::util;

TEST_CASE("GLTF Load a cube with averaged normals") {
    auto const meshData = loadGltfFile(getProjectRoot() / "content/models/cube.glb",
                                       {NormalTreatment::AVERAGE, TexCoordTreatment::DISCARD});

    REQUIRE(meshData.mVertices.size() == 8);
    auto const expectedNormalComponent = 1.f / std::sqrt(3.f);

    for (auto const &v: meshData.mVertices) {
        for (size_t c = 0; c < 3; c++) {
            CHECK(abs(v.mPosition[c]) == 1.f);
            CHECK_THAT(abs(v.mNormal[c]), Catch::Matchers::WithinAbs(expectedNormalComponent, 0.001f));
            CHECK(glm::sign(v.mPosition[c]) == glm::sign(v.mNormal[c]));
        }
    }

    auto const indexVector = meshData.mIndices.asVector<uint16_t>();
    REQUIRE(indexVector.size() == 36);
}

TEST_CASE("GLTF Load a cube with split normals") {
    auto const meshData = loadGltfFile(getProjectRoot() / "content/models/cube.glb",
                                       {NormalTreatment::SPLIT, TexCoordTreatment::DISCARD});
    REQUIRE(meshData.mVertices.size() == 24);

    REQUIRE(meshData.mVertices.at(0).mPosition == glm::vec3(1, 1, 1));
    REQUIRE(meshData.mVertices.at(0).mNormal == glm::vec3(0, 0, 1));
    REQUIRE(meshData.mVertices.at(0).mColor == glm::vec4(1, 1, 1, 1));

    auto const uniquePos = gfx_testing::test::getUniqueVertexPositions(meshData);
    REQUIRE(uniquePos.contains({1, 1, 1}));
    REQUIRE(uniquePos.contains({-1, 1, 1}));
    REQUIRE(uniquePos.contains({1, -1, 1}));
    REQUIRE(uniquePos.contains({-1, -1, 1}));
    REQUIRE(uniquePos.contains({1, 1, -1}));
    REQUIRE(uniquePos.contains({-1, 1, -1}));
    REQUIRE(uniquePos.contains({1, -1, -1}));
    REQUIRE(uniquePos.contains({-1, -1, -1}));

    auto const indexVector = meshData.mIndices.asVector<uint16_t>();
    REQUIRE(indexVector.size() == 36);
}
