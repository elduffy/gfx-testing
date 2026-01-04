#pragma once
#include <ecs/ecs.hpp>
#include <render/render_object.hpp>
#include <util/cube_map.hpp>

namespace gfx_testing::render {
    class SkyBox {
    public:
        static SkyBox &create(ecs::Ecs &ecs, game::GameContext &gameContext, util::CubeMap cubeMap);

        SkyBox(ecs::EntityId entityId, game::GameContext &gameContext, util::CubeMap cubeMap);

        ecs::EntityId mEntityId;
        RenderObject &mRenderObject;
    };
} // namespace gfx_testing::render
