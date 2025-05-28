#pragma once
#include <sdl.hpp>
#include <util/util.hpp>


namespace gfx_testing::pipeline {

    class PipelineBuffers {
    public:
        NO_COPY(PipelineBuffers);

        explicit PipelineBuffers(std::vector<sdl::SdlGpuBuffer> storageBuffers) :
            mStorageBuffers(std::move(storageBuffers)), mStorageBufferPtrs(mStorageBuffers.size(), nullptr) {
            for (int i = 0; i < mStorageBuffers.size(); ++i) {
                mStorageBufferPtrs.at(i) = *mStorageBuffers.at(i);
            }
        }

        PipelineBuffers(PipelineBuffers &&other) noexcept :
            mStorageBuffers(std::move(other.mStorageBuffers)), mStorageBufferPtrs(other.mStorageBufferPtrs) {}

    private:
        std::vector<sdl::SdlGpuBuffer> mStorageBuffers;

    public:
        std::vector<SDL_GPUBuffer *> mStorageBufferPtrs;
    };
} // namespace gfx_testing::pipeline
