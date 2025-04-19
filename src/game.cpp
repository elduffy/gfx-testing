#include <game.hpp>

namespace gfx_testing::game {
    GameContext::GameContext(sdl::SdlContext const &sdlContext):
        mSdlContext(sdlContext) {
        SDL_Log("GameContext initialized");
    }
}
