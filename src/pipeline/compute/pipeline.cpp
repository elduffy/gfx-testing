#include <pipeline/compute/pipeline.hpp>


namespace gfx_testing::pipeline::compute {

    Pipeline::Pipeline(PipelineDefinition const &definition, sdl::SdlComputePipeline sdlPipeline) :
        mDefinition(definition), mSdlPipeline(std::move(sdlPipeline)) {}

} // namespace gfx_testing::pipeline::compute
