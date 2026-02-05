#include <SDL3/SDL_log.h>
#include <absl/log/check.h>
#include <debug/imgui_context.hpp>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <render/render_object.hpp>
#include <sdl.hpp>


namespace gfx_testing::imgui {
    // Set true to show the demo menu to try out widgets, etc
    static auto constexpr SHOW_DEMO = true;

    float *getVectorData(glm::vec3 &v) {
        static_assert(offsetof(glm::vec3, x) == 0);
        static_assert(sizeof(v.x) == sizeof(float));
        static_assert(offsetof(glm::vec3, y) == sizeof(float));
        static_assert(sizeof(v.y) == sizeof(float));
        static_assert(offsetof(glm::vec3, z) == offsetof(glm::vec3, y) + sizeof(float));
        static_assert(sizeof(v.z) == sizeof(float));
        return &v.x;
    }

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
        CHECK(ImGui_ImplSDLGPU3_Init(&init_info)) << "Failed to initialize ImGui";
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

    void ImGuiContext::showDebugWindow(render::Scene &scene) {
        ImGuiWindowFlags windowFlags = 0;
        windowFlags |= ImGuiWindowFlags_NoNav;
        if (!ImGui::Begin("Debug", &mOpenWindow, windowFlags)) {
            ImGui::End();
            return;
        }
        const ImGuiIO &io = ImGui::GetIO();
        auto const &gameSettings = scene.getGameContext().mGameSettings;

        if (ImGui::CollapsingHeader("Perf")) {
            if (auto const &targetFps = gameSettings.mTargetFps; targetFps.has_value()) {
                ImGui::Text("FPS target is %.1f", targetFps.value());
            } else if (gameSettings.mVsyncDisabled) {
                ImGui::Text("FPS is uncapped");
            } else {
                ImGui::Text("FPS is capped by vsync");
            }
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        }

        if (ImGui::CollapsingHeader("ECS")) {
            auto &ecs = scene.getGameContext().getEcs();
            mEcsView.render(ecs);
        }

        if (ImGui::CollapsingHeader("Camera")) {
            auto &camera = scene.getCamera();
            glm::vec3 cameraPos = camera.getPosition();
            glm::vec3 cameraPivot = camera.getPivot();
            auto *position = getVectorData(cameraPos);

            ImGui::DragFloat3("Position", position);

            auto *pivot = getVectorData(cameraPivot);
            ImGui::DragFloat3("Pivot", pivot);
            ImGui::SameLine();
            if (ImGui::Button("Reset")) {
                memset(pivot, 0.f, 3 * sizeof(float));
            }


            camera.setPosition(cameraPos);
            camera.setPivot(cameraPivot);
        }

        if (ImGui::CollapsingHeader("Debug")) {
            // Debug Normals
            {
                auto &sceneObjects = scene.getSceneObjects();
                bool normalsEnabled = sceneObjects.hasDebugNormals();
                ImGui::Checkbox("Debug Normals", &normalsEnabled);
                sceneObjects.toggleDebugNormals(normalsEnabled);
            }
        }
        ImGui::End();
    }

    void ImGuiContext::renderFrame(render::DrawContext const &drawContext, render::Scene &scene) {
        if (!mOpenWindow) {
            return;
        }
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        if constexpr (SHOW_DEMO) {
            ImGui::ShowDemoWindow(&mOpenWindow);
        }
        showDebugWindow(scene);
        ImGui::Render();

        auto *drawData = ImGui::GetDrawData();
        Imgui_ImplSDLGPU3_PrepareDrawData(drawData, *drawContext.mCommandBuffer);

        const SDL_GPUColorTargetInfo colorTargetInfo{
                .texture = drawContext.mSwapchainTexture,
                .clear_color = {0, 0, 0, 1},
                .load_op = SDL_GPU_LOADOP_LOAD,
                .store_op = SDL_GPU_STOREOP_STORE,
        };
        SDL_GPURenderPass *renderPass =
                SDL_BeginGPURenderPass(*drawContext.mCommandBuffer, &colorTargetInfo, 1, nullptr);
        ImGui_ImplSDLGPU3_RenderDrawData(drawData, *drawContext.mCommandBuffer, renderPass);
        SDL_EndGPURenderPass(renderPass);
    }
} // namespace gfx_testing::imgui
