#pragma once

#include <debug/imgui_debug_view.hpp>
#include <render/draw_context.hpp>
#include <render/scene.hpp>
#include <sdl.hpp>

namespace gfx_testing::imgui {
    class ImGuiContext {
    public:
        NO_COPY_NO_MOVE(ImGuiContext);

        ~ImGuiContext();

        explicit ImGuiContext(sdl::SdlContext const &sdlContext);

        void toggleOpen() { mOpenWindow = !mOpenWindow; }

        /**
         * @return true if the event was captured by imgui
         */
        bool processEvent(SDL_Event const &sdlEvent) const;

        void renderFrame(render::DrawContext const &drawContext, render::Scene &scene);

    private:
        void showDebugWindow(render::Scene &scene);

        bool mOpenWindow{false};

        std::vector<std::unique_ptr<ImguiDebugView>> mDebugViews;
    };
} // namespace gfx_testing::imgui
