#include <game.hpp>

namespace gfx_testing::game {
    GameContext::GameContext(sdl::SdlContext const &sdlContext, io::ResourceLoader const &resourceLoader,
                             GameSettings const &settings) :
        mSdlContext(sdlContext), mResourceLoader(resourceLoader), mPipelines(sdlContext, resourceLoader),
        mSamplers(sdlContext), mGameSettings(settings), mFpsCapper(settings.mTargetFps) {
        SDL_Log("GameContext initialized");
    }
} // namespace gfx_testing::game
