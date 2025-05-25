
#include <io/gltf_loader.hpp>
#undef CHECK // Use the Catch2 CHECK in this file
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../util.hpp"

using namespace gfx_testing::util;

TEST_CASE("GLTF Load a cube with averaged normals") {
    auto const [meshData, images] = gfx_testing::io::loadGltfFile(
            getProjectRoot() / "content/models/cube.glb", {NormalTreatment::AVERAGE, TexCoordTreatment::DISCARD});

    REQUIRE(meshData.mVertices.size() == 8);
    REQUIRE(images.empty());
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
    auto const [meshData, images] = gfx_testing::io::loadGltfFile(getProjectRoot() / "content/models/cube.glb",
                                                                  {NormalTreatment::SPLIT, TexCoordTreatment::DISCARD});
    REQUIRE(meshData.mVertices.size() == 24);
    REQUIRE(images.empty());

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

TEST_CASE("GLTF Load a textured cube with averaged normals") {
    auto const [meshData, images] = gfx_testing::io::loadGltfFile(
            getProjectRoot() / "content/models/tex-cube.glb", {NormalTreatment::AVERAGE, TexCoordTreatment::DISCARD});

    CHECK(meshData.mVertices.size() == 8);
    CHECK(meshData.mIndices.count() == 36);
    REQUIRE(images.size() == 1);
    CHECK(images.front().mTextureType == SDL_GPU_TEXTURETYPE_2D);
    CHECK(images.front().mSurfaces.size() == 1);
    REQUIRE(images.front().mSamplerCreateInfo.has_value());
    auto const &createInfo = images.front().mSamplerCreateInfo.value();
    CHECK(createInfo.min_filter == SDL_GPU_FILTER_LINEAR);
    CHECK(createInfo.mag_filter == SDL_GPU_FILTER_LINEAR);
    CHECK(createInfo.mipmap_mode == SDL_GPU_SAMPLERMIPMAPMODE_LINEAR);
    CHECK(createInfo.address_mode_u == SDL_GPU_SAMPLERADDRESSMODE_REPEAT);
    CHECK(createInfo.address_mode_v == SDL_GPU_SAMPLERADDRESSMODE_REPEAT);
    CHECK(createInfo.enable_anisotropy == true);
}
