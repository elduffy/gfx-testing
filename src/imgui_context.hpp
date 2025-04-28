#pragma once
#include "sdl.hpp"

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

        void renderFrame(SDL_GPUCommandBuffer *commandBuffer, SDL_GPUColorTargetInfo const &colorTargetInfo);

    private:
        void showDebugWindow();

        bool mOpenWindow{false};
    };
}
