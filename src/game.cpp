#include <game.hpp>

namespace gfx_testing::game {
    GameContext::GameContext(sdl::SdlContext const &sdlContext, util::ResourceLoader const &resourceLoader):
        mSdlContext(sdlContext), mResourceLoader(resourceLoader), mPipelines(sdlContext, resourceLoader) {
        SDL_Log("GameContext initialized");
    }
}
