#include <game.hpp>

namespace gfx_testing::game {

    std::vector<SDL_GPUPresentMode> GameSettings::getPresentModes() const {
        std::vector<SDL_GPUPresentMode> presentModes;
        if (mTargetFps.has_value()) {
            presentModes.push_back(SDL_GPU_PRESENTMODE_MAILBOX);
            presentModes.push_back(SDL_GPU_PRESENTMODE_IMMEDIATE);
        } else if (mVsyncDisabled) {
            presentModes.push_back(SDL_GPU_PRESENTMODE_IMMEDIATE);
        } else {
            presentModes.push_back(SDL_GPU_PRESENTMODE_VSYNC);
        }
        return presentModes;
    }

    std::vector<SDL_GPUSwapchainComposition> GameSettings::getSwapchainCompositions() const {
        std::vector<SDL_GPUSwapchainComposition> swapchainCompositions;
        swapchainCompositions.push_back(SDL_GPU_SWAPCHAINCOMPOSITION_SDR);
        if (mHdrEnabled) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "HDR support is untested.");
            swapchainCompositions.push_back(SDL_GPU_SWAPCHAINCOMPOSITION_HDR_EXTENDED_LINEAR);
            swapchainCompositions.push_back(SDL_GPU_SWAPCHAINCOMPOSITION_HDR10_ST2084);
        }
        return swapchainCompositions;
    }

    GameContext::GameContext(sdl::SdlContext const &sdlContext, io::ResourceLoader const &resourceLoader,
                             GameSettings const &settings) :
        mSdlContext(sdlContext), mResourceLoader(resourceLoader), mPipelines(sdlContext, resourceLoader),
        mSamplers(sdlContext), mGameSettings(settings), mFpsCapper(settings.mTargetFps) {
        SDL_Log("GameContext initialized");
    }
} // namespace gfx_testing::game
