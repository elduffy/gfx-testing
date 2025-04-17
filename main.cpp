#include <filesystem>
#include <SDL3/SDL.h>
#include <obj_loader.hpp>
#include <sdl.hpp>
#include <util.hpp>

void handleEvent(SDL_Event const &event) {
    // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
}

int main() {
    gfx_testing::sdl::SdlContext context;
    auto projectRoot = gfx_testing::util::getProjectRoot();
    gfx_testing::model::loadObjFile(projectRoot / "content/models/basic-shapes.obj");
    auto shader = gfx_testing::util::loadShader(context,
                                                projectRoot / "content/shaders/src/pos_color_transform.vert.hlsl", 0,
                                                1, 0, 0);
    gfx_testing::sdl::runEventLoop(handleEvent);
    return 0;
}
