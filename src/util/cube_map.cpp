#include <absl/log/check.h>
#include <boost/safe_numerics/checked_default.hpp>
#include <util/cube_map.hpp>

namespace gfx_testing::util {
    CubeMap::CubeMap(std::vector<sdl::SdlSurface> surfaces) :
        mSurfaces(std::move(surfaces)), mExtent(mSurfaces.front().getExtent()) {
        for (auto const &surface: mSurfaces) {
            CHECK(surface.getExtent() == mExtent)
                    << "Cube map extent mismatch: " << surface.getExtent() << " != " << mExtent;
        }
    }

    void transferTextureData(sdl::SdlContext const &context, std::vector<sdl::SdlSurface> const &surfaces,
                             sdl::SdlGpuTexture const &texture) {
        {
            CHECK_EQ(surfaces.size(), 6);
            const auto *surface = surfaces.front().mSurface;
            CHECK_EQ(surface->pitch, surface->w * 4);
        }
        auto const extent = surfaces.front().getExtent();
        auto const bytesPerLayer = extent.mWidth * extent.mHeight * 4;
        const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size = bytesPerLayer * 6,
        };
        const sdl::SdlTransferBuffer transferBuffer{
                context, SDL_CreateGPUTransferBuffer(context.mDevice, &transferBufferCreateInfo)};
        // Upload pixel data
        {
            const auto mappedBuffer = transferBuffer.map(false);
            for (uint32_t layer = 0; layer < surfaces.size(); layer++) {
                const auto *source = surfaces[layer].mSurface->pixels;
                auto *dest = mappedBuffer.get<uint8_t>(bytesPerLayer * layer);
                SDL_memcpy(dest, source, bytesPerLayer);
            }
        }

        auto *commandBuffer = SDL_AcquireGPUCommandBuffer(context.mDevice);
        if (commandBuffer == nullptr) {
            throw std::runtime_error("Failed to acquire GPU command buffer");
        }
        auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);
        auto *copyPass = SDL_BeginGPUCopyPass(commandBuffer);
        SDL_GPUTextureTransferInfo source = {
                .transfer_buffer = *transferBuffer,
                .rows_per_layer = extent.mHeight,
        };
        SDL_GPUTextureRegion dest = {
                .w = extent.mWidth,
                .h = extent.mHeight,
                .d = 1,
        };
        for (uint32_t face = 0; face < 6; ++face) {
            auto const &surface = surfaces.at(face);
            source.pixels_per_row = boost::safe_numerics::checked::cast<uint32_t>((*surface)->w),
            source.offset = face * bytesPerLayer;
            dest.texture = *texture;
            dest.layer = face;
            SDL_UploadToGPUTexture(copyPass, &source, &dest, false);
        }
        SDL_EndGPUCopyPass(copyPass);
    }

    sdl::SdlGpuTexture CubeMap::createTexture(sdl::SdlContext const &context) const {
        const SDL_GPUTextureCreateInfo createInfo{
                .type = SDL_GPU_TEXTURETYPE_CUBE,
                .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                .width = mExtent.mWidth,
                .height = mExtent.mHeight,
                .layer_count_or_depth = 6,
                .num_levels = 1,
        };
        auto *texture = SDL_CreateGPUTexture(context.mDevice, &createInfo);
        if (texture == nullptr) {
            throw std::runtime_error("Failed to create texture");
        }
        sdl::SdlGpuTexture result{context, texture};
        transferTextureData(context, mSurfaces, result);
        return result;
    }

} // namespace gfx_testing::util
