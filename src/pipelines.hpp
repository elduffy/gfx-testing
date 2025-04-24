#pragma once

#include <sdl.hpp>

#include "resource_loader.hpp"

namespace gfx_testing::pipeline {
    class Pipelines {
        Pipelines(sdl::SdlContext const &sdlContext, SDL_GPUShader *defaultVertexShader,
                  SDL_GPUShader *normColorFragShader,
                  SDL_GPUShader *goochFragShader);

        Pipelines(sdl::SdlContext const &sdlContext, util::ShaderCode const &defaultVertexShader,
                  util::ShaderCode const &normColorFragShader,
                  util::ShaderCode const &goochFragShader);

    public:
        Pipelines(sdl::SdlContext const &sdlContext, util::ResourceLoader const &resourceLoader);

        sdl::SdlGfxPipeline mDiffuse;
        sdl::SdlGfxPipeline mGooch;
    };
}
