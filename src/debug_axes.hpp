#pragma once
#include "game.hpp"
#include "render_object.hpp"

namespace gfx_testing::render {

    class DebugAxes {
    public:
        DebugAxes(game::GameContext const &gameContext, float axisLength);

        RenderObject mRenderObject;
    };

}
