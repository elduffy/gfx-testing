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

    shader::ShaderObject createShaderObject(game::GameContext const &gameContext, util::CubeMap const &cubeMap) {
        auto texture = cubeMap.createTexture(gameContext.mSdlContext);
        shader::RenderResources renderResources;
        renderResources.mTextures.emplace_back(cubeMap.createTexture(gameContext.mSdlContext),
                                               gameContext.mSamplers.mAnisotropicWrap);
        return {createMeshData(), (std::move(renderResources))};
    }


    SkyBox::SkyBox(game::GameContext &gameContext, util::CubeMap const &cubeMap) :
        mGameContext(gameContext), mRenderObject(gameContext, createShaderObject(mGameContext, cubeMap),
                                                 pipeline::PipelineName::Skybox, glm::identity<glm::mat4>()) {}
} // namespace gfx_testing::render
