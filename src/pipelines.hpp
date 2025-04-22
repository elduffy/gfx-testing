#pragma once

#include <sdl.hpp>

namespace gfx_testing::pipeline {
    class Pipelines {
        Pipelines(sdl::SdlContext const &sdlContext, SDL_GPUShader *defaultVertexShader,
                  SDL_GPUShader *normColorFragShader,
                  SDL_GPUShader *goochFragShader);

    public:
        Pipelines(sdl::SdlContext const &sdlContext, std::filesystem::path const &projectRoot);

        sdl::SdlGfxPipeline mDiffuse;
        sdl::SdlGfxPipeline mGooch;
    };
}
