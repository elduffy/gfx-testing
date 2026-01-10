#pragma once
#include <ecs/ecs.hpp>
#include <game.hpp>
#include <render/render_object.hpp>

namespace gfx_testing::render {

    class DebugAxes {
    public:
        static DebugAxes &create(ecs::Ecs &ecs, game::GameContext &gameContext);

        explicit DebugAxes(ecs::EntityId entityId, game::GameContext &gameContext);

        RenderObject &mRenderObject;
    };

} // namespace gfx_testing::render
