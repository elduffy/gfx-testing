#include <pipeline/compute/pipeline.hpp>


namespace gfx_testing::pipeline::compute {

    std::vector<sdl::SdlGpuBuffer> allocateStorageBuffers(sdl::SdlContext const &,
                                                          PipelineDefinition const &pipelineDefinition) {
        std::vector<sdl::SdlGpuBuffer> result;
        switch (pipelineDefinition.mName) {
            case PipelineName::DebugNormals:
                break;
        }
        return result;
    }

    Pipeline::Pipeline(PipelineDefinition const &definition, sdl::SdlComputePipeline sdlPipeline) :
        mDefinition(definition), mSdlPipeline(std::move(sdlPipeline)),
        mBuffers(allocateStorageBuffers(sdlPipeline.mContext, mDefinition)) {}

    void Pipeline::bindStorageBuffers(SDL_GPUComputePass *computePass) const {
        SDL_BindGPUComputeStorageBuffers(computePass, 0, mBuffers.mStorageBufferPtrs.data(),
                                         mBuffers.mStorageBufferPtrs.size());
    }
} // namespace gfx_testing::pipeline::compute
