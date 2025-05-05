#pragma once
#include <game.hpp>
#include <render/render_object.hpp>

namespace gfx_testing::render {

    class DebugAxes {
    public:
        explicit DebugAxes(game::GameContext const &gameContext);

        RenderObject mRenderObject;
    };

}
