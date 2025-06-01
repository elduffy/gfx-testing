#pragma once

#include <io/resource_loader.hpp>
#include <map>
#include <pipeline/compute/pipeline.hpp>
#include <pipeline/gfx/pipeline.hpp>
#include <sdl.hpp>

namespace gfx_testing::pipeline {
    static constexpr auto MSAA_SAMPLE_COUNT = SDL_GPU_SAMPLECOUNT_4;


    class Pipelines {
        Pipelines(sdl::SdlContext const &sdlContext, std::map<gfx::ShaderDefinition, sdl::SdlShader> const &gfxShaders,
                  std::map<compute::ShaderDefinition, io::ShaderCode> const &computeShaderCode);

        Pipelines(sdl::SdlContext const &sdlContext,
                  std::map<gfx::ShaderDefinition, io::ShaderCode> const &gfxShaderCode,
                  std::map<compute::ShaderDefinition, io::ShaderCode> const &computeShaderCode);

    public:
        Pipelines(sdl::SdlContext const &sdlContext, io::ResourceLoader const &resourceLoader);

        gfx::Pipeline const &get(gfx::PipelineName pipelineName) const {
            return mGfxPipelines.at(getIndex(pipelineName));
        }

        compute::Pipeline const &get(compute::PipelineName pipelineName) const {
            return mComputePipelines.at(getIndex(pipelineName));
        }

    private:
        std::vector<gfx::Pipeline> mGfxPipelines;
        std::vector<compute::Pipeline> mComputePipelines;
    };
} // namespace gfx_testing::pipeline
