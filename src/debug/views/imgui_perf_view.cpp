#include <debug/views/imgui_perf_view.hpp>
#include <imgui.h>

void gfx_testing::imgui::ImGuiPerfView::render(render::Scene &scene) {
    const ImGuiIO &io = ImGui::GetIO();
    auto const &gameSettings = scene.getGameContext().mGameSettings;
    if (auto const &targetFps = gameSettings.mTargetFps; targetFps.has_value()) {
        ImGui::Text("FPS target is %.1f", targetFps.value());
    } else if (gameSettings.mVsyncDisabled) {
        ImGui::Text("FPS is uncapped");
    } else {
        ImGui::Text("FPS is capped by vsync");
    }
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
}
