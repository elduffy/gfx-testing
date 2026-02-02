#include <ecs/render_ecs.hpp>
#include <game.hpp>
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

    shader::ShaderObject createShaderObject(util::CubeMap cubeMap) {
        std::vector<shader::ImageData> images;
        images.emplace_back(SDL_GPU_TEXTURETYPE_CUBE, std::move(cubeMap.mSurfaces));
        return {createMeshData(), std::move(images)};
    }

    RenderObject &createRenderObject(ecs::EntityId entityId, game::GameContext &gameContext, util::CubeMap cubeMap) {
        return ecs::render::emplaceRenderObject<pipeline::gfx::PipelineName::Skybox>(
                entityId, gameContext, createShaderObject(std::move(cubeMap)), pipeline::gfx::PipelineName::Skybox,
                glm::identity<glm::mat4>());
    }


    SkyBox &SkyBox::create(ecs::Ecs &ecs, game::GameContext &gameContext, util::CubeMap cubeMap) {
        auto entityId = ecs.create("SkyBox");
        return entityId.emplace<SkyBox>(entityId, gameContext, std::move(cubeMap));
    }


    SkyBox::SkyBox(ecs::EntityId entityId, game::GameContext &gameContext, util::CubeMap cubeMap) :
        mEntityId(entityId), mRenderObject(createRenderObject(entityId, gameContext, std::move(cubeMap))) {}
} // namespace gfx_testing::render
