#pragma once
#include <render/render_object.hpp>
#include <util/cube_map.hpp>

namespace gfx_testing::render {
    class SkyBox {
    public:
        SkyBox(game::GameContext &gameContext, util::CubeMap cubeMap);

        RenderObject mRenderObject;
    };
} // namespace gfx_testing::render
