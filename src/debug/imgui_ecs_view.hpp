#pragma once

#include <debug/imgui_debug_view.hpp>

namespace gfx_testing::imgui {
    class ImGuiEcsView : public ImguiDebugView {
    public:
        ~ImGuiEcsView() override = default;

        const char *getName() override { return "ECS"; }

        void render(render::Scene &scene) override;
    };
} // namespace gfx_testing::imgui
