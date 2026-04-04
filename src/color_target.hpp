#pragma once
#include <SDL3/SDL_gpu.h>
#include <sdl.hpp>
#include <util/util.hpp>

namespace gfx_testing::sdl {
    class ColorTargetSource {
    public:
        explicit ColorTargetSource(SdlContext const &sdlContext);

        SDL_GPUTexture *acquireColorTarget(SDL_GPUCommandBuffer *commandBuffer, util::Extent2D &extentOut) const;

    private:
        SdlContext const &mSdlContext;
        SdlGpuTexture mHeadlessColorTarget;
    };
} // namespace gfx_testing::sdl
