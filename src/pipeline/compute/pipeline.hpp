#pragma once

#include <pipeline/buffers.hpp>
#include <pipeline/compute/pipeline_definition.hpp>
#include <sdl.hpp>

namespace gfx_testing::pipeline::compute {
    // TODO: can this be merged with gfx::Pipeline?
    class Pipeline {
    public:
        NO_COPY(Pipeline);

        Pipeline(PipelineDefinition const &definition, sdl::SdlComputePipeline sdlPipeline);

        Pipeline(Pipeline &&other) = default;

        PipelineDefinition const &mDefinition;
        sdl::SdlComputePipeline mSdlPipeline;
    };
} // namespace gfx_testing::pipeline::compute
