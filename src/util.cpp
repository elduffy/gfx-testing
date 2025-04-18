#include <algorithm>
#include <format>
#include <sdl.hpp>
#include <util.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <SDL3/SDL_log.h>
#include <boost/scope/scope_exit.hpp>

namespace gfx_testing::util {
    std::filesystem::path getProjectRoot() {
        std::filesystem::path path = std::filesystem::current_path();
        while (!is_empty(path)) {
            if (path.filename() == "gfx-testing") {
                return path;
            }
            path = path.parent_path();
        }
        throw std::runtime_error("Could not find project root directory.");
    }

    sdl::SdlShader loadShader(
            sdl::SdlContext const &sdlContext,
            const std::filesystem::path &shaderSourcePath,
            uint32_t samplers,
            uint32_t uniformBuffers,
            uint32_t storageBuffers,
            uint32_t storageTextures
            ) {
        SDL_GPUShaderStage stage;
        if (SDL_strstr(shaderSourcePath.filename().c_str(), ".vert")) {
            stage = SDL_GPU_SHADERSTAGE_VERTEX;
        } else if (SDL_strstr(shaderSourcePath.filename().c_str(), ".frag")) {
            stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
        } else {
            throw std::runtime_error("Could not determine shader stage for file.");
        }

        auto backendFormats = SDL_GetGPUShaderFormats(sdlContext.mDevice);
        if (!(backendFormats & SDL_GPU_SHADERFORMAT_SPIRV)) {
            throw std::runtime_error("Only supporting SPIRV formats for now");
        }

        std::string compiledFilename = shaderSourcePath.filename().string();
        boost::replace_last(compiledFilename, ".hlsl", ".spv");
        auto compiledFilePath = shaderSourcePath.parent_path().parent_path() / "compiled/spirv" / compiledFilename;


        size_t codeSize = 0;
        auto *code = static_cast<uint8_t *>(SDL_LoadFile(compiledFilePath.c_str(), &codeSize));
        boost::scope::scope_exit freeCodeGuard([&code] {
            SDL_free(code);
        });
        SDL_Log("Loaded SPIRV shader from %s of size %zu", compiledFilePath.c_str(), codeSize);

        if (code == nullptr) {
            throw std::runtime_error("Could not load SPIRV shader from file.");
        }

        const SDL_GPUShaderCreateInfo shaderCreateInfo{
                .code_size = codeSize,
                .code = code,
                .entrypoint = "main",
                .format = SDL_GPU_SHADERFORMAT_SPIRV,
                .stage = stage,
                .num_samplers = samplers,
                .num_storage_textures = storageTextures,
                .num_storage_buffers = storageBuffers,
                .num_uniform_buffers = uniformBuffers,
        };
        SDL_GPUShader *shader = SDL_CreateGPUShader(sdlContext.mDevice, &shaderCreateInfo);
        if (shader == nullptr) {
            throw std::runtime_error("Could not create shader.");
        }
        return {sdlContext, shader};
    }
}
