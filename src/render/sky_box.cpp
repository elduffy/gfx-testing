#include <game.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <render/sky_box.hpp>

namespace gfx_testing::render {
    shader::MeshData createMeshData() {
        constexpr auto LEN = 100.f;
        shader::MeshDataBuilder builder;

        builder.mVertices.emplace_back(glm::vec3(-LEN, -LEN, -LEN));
        builder.mVertices.emplace_back(glm::vec3(LEN, -LEN, -LEN));
        builder.mVertices.emplace_back(glm::vec3(LEN, LEN, -LEN));
        builder.mVertices.emplace_back(glm::vec3(-LEN, LEN, -LEN));

        builder.mVertices.emplace_back(glm::vec3(-LEN, -LEN, LEN));
        builder.mVertices.emplace_back(glm::vec3(LEN, -LEN, LEN));
        builder.mVertices.emplace_back(glm::vec3(LEN, LEN, LEN));
        builder.mVertices.emplace_back(glm::vec3(-LEN, LEN, LEN));

        builder.mVertices.emplace_back(glm::vec3(-LEN, -LEN, -LEN));
        builder.mVertices.emplace_back(glm::vec3(-LEN, LEN, -LEN));
        builder.mVertices.emplace_back(glm::vec3(-LEN, LEN, LEN));
        builder.mVertices.emplace_back(glm::vec3(-LEN, -LEN, LEN));

        builder.mVertices.emplace_back(glm::vec3(LEN, -LEN, -LEN));
        builder.mVertices.emplace_back(glm::vec3(LEN, LEN, -LEN));
        builder.mVertices.emplace_back(glm::vec3(LEN, LEN, LEN));
        builder.mVertices.emplace_back(glm::vec3(LEN, -LEN, LEN));

        builder.mVertices.emplace_back(glm::vec3(-LEN, -LEN, -LEN));
        builder.mVertices.emplace_back(glm::vec3(-LEN, -LEN, LEN));
        builder.mVertices.emplace_back(glm::vec3(LEN, -LEN, LEN));
        builder.mVertices.emplace_back(glm::vec3(LEN, -LEN, -LEN));

        builder.mVertices.emplace_back(glm::vec3(-LEN, LEN, -LEN));
        builder.mVertices.emplace_back(glm::vec3(-LEN, LEN, LEN));
        builder.mVertices.emplace_back(glm::vec3(LEN, LEN, LEN));
        builder.mVertices.emplace_back(glm::vec3(LEN, LEN, -LEN));

        Uint16 indices[] = {0,  1,  2,  0,  2,  3,  6,  5,  4,  7,  6,  4,  8,  9,  10, 8,  10, 11,
                            14, 13, 12, 15, 14, 12, 16, 17, 18, 16, 18, 19, 22, 21, 20, 23, 22, 20};
        for (auto const index: indices) {
            builder.addIndex(index);
        }
        return builder.build();
    }

    SkyBox::SkyBox(game::GameContext &gameContext, util::CubeMap const &cubeMap) :
        mGameContext(gameContext),
        mRenderObject(gameContext, createMeshData(), pipeline::PipelineName::Skybox,
                      cubeMap.createTexture(mGameContext.mSdlContext), glm::identity<glm::mat4>()) {}
} // namespace gfx_testing::render
