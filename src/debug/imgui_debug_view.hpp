#pragma once
#include <render/scene.hpp>

namespace gfx_testing::imgui {
    class ImguiDebugView {
    public:
        virtual ~ImguiDebugView() = default;

        virtual char const *getName() = 0;
        virtual void render(render::Scene &scene) = 0;
    };
} // namespace gfx_testing::imgui
