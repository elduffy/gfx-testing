#pragma once

#include <filesystem>
#include <SDL3/SDL_gpu.h>

namespace gfx_testing::util {
    std::filesystem::path getProjectRoot();

    sdl::SdlShader loadShader(
        sdl::SdlContext const &context,
        const std::filesystem::path &shaderSourcePath,
        uint32_t samplers,
        uint32_t uniformBuffers,
        uint32_t storageBuffers,
        uint32_t storageTextures
    );
}

