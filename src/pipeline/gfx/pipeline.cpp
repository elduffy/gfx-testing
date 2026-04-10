#include <algorithm>

#include <SDL3/SDL_gpu.h>
#include <absl/log/check.h>
#include <pipeline/gfx/pipeline.hpp>
#include <pipeline/gfx/pipeline_definition.hpp>
#include <pipeline/util.hpp>
#include <sdl.hpp>
#include <shader/shader_models.hpp>

namespace gfx_testing::pipeline::gfx {

    std::vector<sdl::SdlGpuBuffer> allocateStorageBuffers(sdl::SdlContext const &context,
                                                          PipelineDefinition const &pipelineDefinition) {
        std::vector<sdl::SdlGpuBuffer> result;
        switch (pipelineDefinition.mName) {
            case PipelineName::Gooch: {
                constexpr shader::GoochParams params{
                        .mCoolColor = {0, 0, 0.55},
                        .mWarmColor = {0.3, 0.3, 0},
                };
                uploadParams(result, context, SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ, params);
                break;
            }
            case PipelineName::Lambert: {
                constexpr shader::LambertParams params{
                        .mUnlitColor = {.1, .1, .1},
                        .mLitColor = {.3, .3, .3},
                };
                uploadParams(result, context, SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ, params);
                break;
            }
            case PipelineName::SimpleColor:
            case PipelineName::Textured:
            case PipelineName::Lines:
            case PipelineName::Skybox:
                break;
        }
        return result;
    }

    Pipeline::Pipeline(PipelineDefinition const &definition, sdl::SdlGfxPipeline sdlPipeline) :
        mDefinition(definition), mSdlPipeline(std::move(sdlPipeline)),
        mBuffers(allocateStorageBuffers(sdlPipeline.mContext, mDefinition)) {}

    void Pipeline::bindStorageBuffers(SDL_GPURenderPass *renderPass) const {
        SDL_BindGPUFragmentStorageBuffers(renderPass, 0, mBuffers.mStorageBufferPtrs.data(),
                                          mBuffers.mStorageBufferPtrs.size());
    }
} // namespace gfx_testing::pipeline::gfx
