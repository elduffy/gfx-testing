#include <filesystem>
#include <SDL3/SDL.h>
#include <obj_loader.hpp>
#include <sdl.hpp>
#include <util.hpp>
#include <shader_models.hpp>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

void handleEvent(SDL_Event const &event) {
    // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
}

gfx_testing::shader::MVPMatrix createMVPMatrix(gfx_testing::sdl::SdlContext const &context) {
    auto const proj = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(context.mWidth) / static_cast<float>(context.mHeight),
        0.1f,
        100.0f
    );
    auto const view = glm::lookAt(
        glm::vec3(5, 5, 5),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );
    auto const model = glm::mat4(1.0f);
    return {proj * view * model};
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
    auto mvpMatrix = createMVPMatrix(context);
}

int main() {
    gfx_testing::sdl::SdlContext context;
    const auto projectRoot = gfx_testing::util::getProjectRoot();
    createScene(context, projectRoot);
    gfx_testing::sdl::runEventLoop(handleEvent);
    return 0;
}
