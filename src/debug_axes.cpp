#include "debug_axes.hpp"

#include "shader_models.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace gfx_testing::render {
    void createLines(shader::MeshDataBuilder &builder, float axisLength) {
        constexpr glm::vec4 RED(1.0f, 0.0f, 0.0f, 1.0f);
        constexpr glm::vec4 GREEN(0.0f, 1.0f, 0.0f, 1.0f);
        constexpr glm::vec4 BLUE(0.0f, 0.0f, 1.0f, 1.0f);

        builder.mVertices.push_back({
                .mPosition = glm::zero<glm::vec3>(),
                .mColor = RED,
        });
        builder.mVertices.push_back({
                .mPosition = {axisLength, 0, 0},
                .mColor = RED,
        });
        builder.mVertices.push_back({
                .mPosition = glm::zero<glm::vec3>(),
                .mColor = GREEN,
        });
        builder.mVertices.push_back({
                .mPosition = {0, axisLength, 0},
                .mColor = GREEN,
        });
        builder.mVertices.push_back({
                .mPosition = glm::zero<glm::vec3>(),
                .mColor = BLUE,
        });
        builder.mVertices.push_back({
                .mPosition = {0, 0, axisLength},
                .mColor = BLUE,
        });

        for (auto i = 0; i < builder.mVertices.size(); i++) {
            builder.addIndex(i);
        }

    }

    shader::MeshData buildMesh(float axisLength) {
        shader::MeshDataBuilder builder;
        createLines(builder, axisLength);
        // TODO: arrows, +, -
        return builder.build();
    }

    DebugAxes::DebugAxes(game::GameContext const &gameContext, float axisLength):
        mRenderObject(gameContext, buildMesh(axisLength), glm::identity<glm::mat4>()) {
    }
}
