#include <buffer.hpp>
#include <sdl.hpp>
#include <shader_models.hpp>

namespace gfx_testing {

    SDL_GPUBuffer *createBuffer(sdl::SdlContext const &context, SDL_GPUBufferUsageFlags usage, uint32_t size) {
        const SDL_GPUBufferCreateInfo createInfo = {
                .usage = usage,
                .size = size,
        };
        return SDL_CreateGPUBuffer(context.mDevice, &createInfo);
    }

    BufferManager::BufferManager(sdl::SdlContext const &context):
        mContext(context) {
    }

    SDL_GPUBuffer *BufferManager::allocate(SDL_GPUBufferUsageFlags usageFlags, uint32_t size) {
        const auto &buffer = mBuffers.emplace_back(mContext, createBuffer(mContext, usageFlags, size));
        return *buffer;
    }

}
