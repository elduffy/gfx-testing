#pragma once
#include <sdl.hpp>
#include <vector>

namespace gfx_testing {
    class BufferManager {
    public:
        explicit BufferManager(sdl::SdlContext const &context);

        SDL_GPUBuffer *allocate(SDL_GPUBufferUsageFlags usageFlags, uint32_t size);

    private:
        sdl::SdlContext const &mContext;
        std::vector<sdl::SdlGpuBuffer> mBuffers;
    };
}
