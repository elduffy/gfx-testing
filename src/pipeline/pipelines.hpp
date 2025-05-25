#pragma once

#include <io/resource_loader.hpp>
#include <map>
#include <pipeline/pipeline.hpp>
#include <pipeline/pipeline_definition.hpp>
#include <sdl.hpp>

namespace gfx_testing::pipeline {
    static constexpr auto MSAA_SAMPLE_COUNT = SDL_GPU_SAMPLECOUNT_4;


    class Pipelines {
        Pipelines(sdl::SdlContext const &sdlContext, std::map<ShaderDefinition, sdl::SdlShader> const &shaders);

        Pipelines(sdl::SdlContext const &sdlContext, std::map<ShaderDefinition, io::ShaderCode> const &shaderCode);

    public:
        Pipelines(sdl::SdlContext const &sdlContext, io::ResourceLoader const &resourceLoader);

        Pipeline const &get(PipelineName pipelineName) const { return mPipelines.at(getIndex(pipelineName)); }

    private:
        std::vector<Pipeline> mPipelines;
    };
} // namespace gfx_testing::pipeline
