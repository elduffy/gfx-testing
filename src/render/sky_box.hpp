#pragma once
#include <render/render_object.hpp>
#include <util/cube_map.hpp>

namespace gfx_testing::render {
    class SkyBox {
    public:
        SkyBox(game::GameContext &gameContext, util::CubeMap const &cubeMap);

        game::GameContext &mGameContext;
        RenderObject mRenderObject;
    };
} // namespace gfx_testing::render
