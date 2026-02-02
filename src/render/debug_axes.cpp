#include <ecs/render_ecs.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <render/debug_axes.hpp>
#include <shader/shader_models.hpp>

namespace gfx_testing::render {
    void createLines(shader::MeshDataBuilder &builder) {
        constexpr auto AXIS_LENGTH = 5.f;
        constexpr auto MAX_TICK_MULTIPLE = 5;
        constexpr auto TICK_SPACING = AXIS_LENGTH / MAX_TICK_MULTIPLE;
        constexpr glm::vec4 RED(1, 0, 0, 1);
        constexpr glm::vec4 GREEN(0, 1, 0, 1);
        constexpr glm::vec4 BLUE(0, 0, 1, 1);
        constexpr glm::vec4 GRAY(.3f, .3f, .3f, 1);

        // Axes
        builder.mVertices.push_back({
                .mPosition = glm::zero<glm::vec3>(),
                .mColor = RED,
        });
        builder.mVertices.push_back({
                .mPosition = {AXIS_LENGTH, 0, 0},
                .mColor = RED,
        });
        builder.mVertices.push_back({
                .mPosition = glm::zero<glm::vec3>(),
                .mColor = GREEN,
        });
        builder.mVertices.push_back({
                .mPosition = {0, AXIS_LENGTH, 0},
                .mColor = GREEN,
        });
        builder.mVertices.push_back({
                .mPosition = glm::zero<glm::vec3>(),
                .mColor = BLUE,
        });
        builder.mVertices.push_back({
                .mPosition = {0, 0, AXIS_LENGTH},
                .mColor = BLUE,
        });

        for (auto tick = -MAX_TICK_MULTIPLE; tick <= MAX_TICK_MULTIPLE; ++tick) {
            auto const tickCoord = tick * TICK_SPACING;
            auto const upperBound = tick == 0 ? 0.f : AXIS_LENGTH;
            builder.mVertices.push_back({
                    .mPosition = {-AXIS_LENGTH, tickCoord, 0},
                    .mColor = GRAY,
            });
            builder.mVertices.push_back({
                    .mPosition = {upperBound, tickCoord, 0},
                    .mColor = GRAY,
            });
            builder.mVertices.push_back({
                    .mPosition = {tickCoord, -AXIS_LENGTH, 0},
                    .mColor = GRAY,
            });
            builder.mVertices.push_back({
                    .mPosition = {tickCoord, upperBound, 0},
                    .mColor = GRAY,
            });
        }


        for (auto i = 0; i < builder.mVertices.size(); i++) {
            builder.addIndex(i);
        }
    }

    shader::ShaderObject buildMesh() {
        shader::MeshDataBuilder builder;
        createLines(builder);
        // TODO: arrows, +, -
        return {builder.build(), {}};
    }

    DebugAxes &DebugAxes::create(ecs::Ecs &ecs, game::GameContext &gameContext) {
        auto entityId = ecs.create("DebugAxes");
        auto &debugAxes = entityId.emplace<DebugAxes>(entityId, gameContext);
        return debugAxes;
    }

    DebugAxes::DebugAxes(ecs::EntityId entityId, game::GameContext &gameContext) :
        mRenderObject(ecs::render::emplaceRenderObject<pipeline::gfx::PipelineName::Lines>(
                entityId, gameContext, buildMesh(), pipeline::gfx::PipelineName::Lines, glm::identity<glm::mat4>())) {}
} // namespace gfx_testing::render
