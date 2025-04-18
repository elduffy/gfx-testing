#include <filesystem>
#include <SDL3/SDL.h>
#include <obj_loader.hpp>
#include <sdl.hpp>
#include <util.hpp>

void handleEvent(SDL_Event const &event) {
    // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
}

void createScene(gfx_testing::sdl::SdlContext const &context, std::filesystem::path const &projectRoot) {
    gfx_testing::model::loadObjFile(projectRoot / "content/models/basic-shapes.obj");
    auto vertexShader = gfx_testing::util::loadShader(context,
                                                      projectRoot / "content/shaders/src/pos_color_transform.vert.hlsl",
                                                      0,
                                                      1, 0, 0);
    auto fragmentShader = gfx_testing::util::loadShader(context,
                                                        projectRoot /
                                                        "content/shaders/src/solid_color.frag.hlsl",
                                                        0,
                                                        0, 0, 0);
}

int main() {
    gfx_testing::sdl::SdlContext context;
    const auto projectRoot = gfx_testing::util::getProjectRoot();
    createScene(context, projectRoot);
    gfx_testing::sdl::runEventLoop(handleEvent);
    return 0;
}
