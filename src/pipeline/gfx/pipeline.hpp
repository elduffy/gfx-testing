#pragma once

#include <pipeline/buffers.hpp>
#include <pipeline/gfx/pipeline_definition.hpp>
#include <sdl.hpp>

namespace gfx_testing::pipeline::gfx {
    class Pipeline {
    public:
        NO_COPY(Pipeline);

        Pipeline(PipelineDefinition const &definition, sdl::SdlPipeline sdlPipeline);

        Pipeline(Pipeline &&other) = default;

        void bindStorageBuffers(SDL_GPURenderPass *renderPass) const;

        PipelineDefinition const &mDefinition;
        sdl::SdlPipeline mSdlPipeline;
        PipelineBuffers mBuffers;
    };
} // namespace gfx_testing::pipeline::gfx
