#pragma once

#include <debug/imgui_debug_view.hpp>

namespace gfx_testing::imgui {

    class ImGuiPerfView : public ImguiDebugView {
    public:
        ~ImGuiPerfView() override = default;

        const char *getName() override { return "Perf"; }

        void render(render::Scene &scene) override;
    };
} // namespace gfx_testing::imgui
