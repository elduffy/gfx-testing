#include <imgui_context.hpp>
#include <SDL3/SDL_log.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlgpu3.h>

#include "sdl.hpp"

namespace gfx_testing::imgui {
    // Set true to show the demo menu to try out widgets, etc
    static auto constexpr SHOW_DEMO = false;

    ImGuiContext::ImGuiContext(sdl::SdlContext const &sdlContext) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
        ImGui_ImplSDL3_InitForSDLGPU(sdlContext.mWindow);
        ImGui_ImplSDLGPU3_InitInfo init_info{
                .Device = sdlContext.mDevice,
                .ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(sdlContext.mDevice, sdlContext.mWindow),
        };
        if (!ImGui_ImplSDLGPU3_Init(&init_info)) {
            throw std::runtime_error("Failed to initialize ImGui");
        }

        SDL_Log("Setup ImGui");
    }

    ImGuiContext::~ImGuiContext() {
        ImGui_ImplSDL3_Shutdown();
        ImGui_ImplSDLGPU3_Shutdown();
        ImGui::DestroyContext();
    }

    bool ImGuiContext::processEvent(SDL_Event const &sdlEvent) const {
        if (!mOpenWindow) {
            return false;
        }
        ImGui_ImplSDL3_ProcessEvent(&sdlEvent);
        const ImGuiIO &io = ImGui::GetIO();
        return io.WantCaptureMouse || io.WantCaptureKeyboard;
    }

    void ImGuiContext::showDebugWindow() {
        ImGuiWindowFlags windowFlags = 0;
        windowFlags |= ImGuiWindowFlags_NoNav;
        if (!ImGui::Begin("Debug", &mOpenWindow, windowFlags)) {
            ImGui::End();
            return;
        }
        const ImGuiIO &io = ImGui::GetIO();

        if (ImGui::CollapsingHeader("Perf")) {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        }
        ImGui::End();
    }

    void ImGuiContext::renderFrame(SDL_GPUCommandBuffer *commandBuffer,
                                   SDL_GPUColorTargetInfo const &colorTargetInfo) {
        if (!mOpenWindow) {
            return;
        }
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        auto const wasOpen = mOpenWindow;
        if constexpr (SHOW_DEMO) {
            ImGui::ShowDemoWindow(&mOpenWindow);
        }
        showDebugWindow();
        if (wasOpen && !mOpenWindow) {
            SDL_Log("Should close window");
        }
        ImGui::Render();

        auto *drawData = ImGui::GetDrawData();
        Imgui_ImplSDLGPU3_PrepareDrawData(drawData, commandBuffer);

        SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);
        ImGui_ImplSDLGPU3_RenderDrawData(drawData, commandBuffer, renderPass);
        SDL_EndGPURenderPass(renderPass);
    }
}
