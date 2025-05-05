#pragma once

#include <map>
#include <sdl.hpp>

#include "pipeline.hpp"
#include "pipeline_definition.hpp"
#include "util/resource_loader.hpp"

namespace gfx_testing::pipeline {
    static constexpr auto MSAA_SAMPLE_COUNT = SDL_GPU_SAMPLECOUNT_4;


    class Pipelines {
        Pipelines(sdl::SdlContext const &sdlContext, std::map<ShaderDefinition, sdl::SdlShader> const &shaders);

        Pipelines(sdl::SdlContext const &sdlContext, std::map<ShaderDefinition, util::ShaderCode> const &shaderCode);

    public:
        Pipelines(sdl::SdlContext const &sdlContext, util::ResourceLoader const &resourceLoader);

        Pipeline const &get(PipelineName pipelineName) const { return mPipelines.at(getIndex(pipelineName)); }

    private:
        std::vector<Pipeline> mPipelines;
    };
} // namespace gfx_testing::pipeline
