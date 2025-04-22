#include <game.hpp>

namespace gfx_testing::game {
    GameContext::GameContext(sdl::SdlContext const &sdlContext, std::string const &projectRoot):
        mSdlContext(sdlContext), mPipelines(sdlContext, projectRoot) {
        SDL_Log("GameContext initialized");
    }
}
