
#include <io/obj_loader.hpp>
#include <iostream>
#include <map>
#include <util/util.hpp>

#undef CHECK // Use the Catch2 CHECK in this file
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace gfx_testing::io;

TEST_CASE("OBJ Load a cube with averaged normals") {
    auto const [meshData, images] =
            loadObjFile(gfx_testing::util::getProjectRoot() / "content/models/cube.obj",
                        {gfx_testing::util::NormalTreatment::AVERAGE, gfx_testing::util::TexCoordTreatment::DISCARD});

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

TEST_CASE("OBJ Load a cube with split normals") {
    auto const [meshData, images] =
            loadObjFile(gfx_testing::util::getProjectRoot() / "content/models/cube.obj",
                        {gfx_testing::util::NormalTreatment::SPLIT, gfx_testing::util::TexCoordTreatment::DISCARD});
    REQUIRE(meshData.mVertices.size() == 24);
    REQUIRE(images.empty());

    REQUIRE(meshData.mVertices.at(0).mPosition == glm::vec3(1, 1, 1));
    REQUIRE(meshData.mVertices.at(0).mNormal == glm::vec3(0, 0, 1));
    REQUIRE(meshData.mVertices.at(0).mColor == glm::vec4(1, 1, 1, 1));

    auto const &v1 = meshData.mVertices.at(23);
    REQUIRE(v1.mPosition == glm::vec3(1, 1, 1));
    REQUIRE(v1.mColor == glm::vec4(1, 1, 1, 1));
    REQUIRE_THAT(v1.mNormal.x, Catch::Matchers::WithinAbs(0, 0.001f));
    REQUIRE_THAT(v1.mNormal.y, Catch::Matchers::WithinAbs(1, 0.001f));
    REQUIRE_THAT(v1.mNormal.z, Catch::Matchers::WithinAbs(0, 0.001f));

    auto const indexVector = meshData.mIndices.asVector<uint16_t>();
    REQUIRE(indexVector.size() == 36);
}
