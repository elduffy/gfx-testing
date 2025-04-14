#include <sdl.hpp>

#include <stdexcept>

namespace gfx_testing::sdl {
    SdlContext::SdlContext(const bool gfxDebug)
        : mWindow(nullptr), mDevice(nullptr) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
            throw std::runtime_error("Failed to initialize SDL");
        }
        SDL_Log("SDL initialized.");

        mWindow = SDL_CreateWindow("gfx-testing", 768, 512, SDL_WINDOW_RESIZABLE);
        if (mWindow == nullptr) {
            SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
            throw std::runtime_error("Failed to create SDL window");
        }
        SDL_Log("Window created.");

        mDevice = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL,
            gfxDebug,
            nullptr
        );
        if (mDevice == nullptr) {
            SDL_Log("Failed to create GPU device: %s", SDL_GetError());
            throw std::runtime_error("Failed to create GPU device");
        }
        SDL_Log("GPU Device created.");

        if (!SDL_ClaimWindowForGPUDevice(mDevice, mWindow)) {
            SDL_Log("Failed to claim window: %s", SDL_GetError());
            throw std::runtime_error("Failed to claim window");
        }
    }

    SdlContext::~SdlContext() {
        SDL_ReleaseWindowFromGPUDevice(mDevice, mWindow);
        SDL_DestroyWindow(mWindow);
        SDL_DestroyGPUDevice(mDevice);
        SDL_Quit();
    }
}
