#pragma once

#include <filesystem>
#include <sdl.hpp>
#include <shader_models.hpp>
#include <util.hpp>
#include <SDL3/SDL.h>

namespace gfx_testing::util {

    class ShaderCode {
    public:
        NO_COPY(ShaderCode);

        explicit ShaderCode(std::filesystem::path const &compiledFilePath, SDL_GPUShaderStage stage):
            mSize(0),
            mCode(static_cast<uint8_t *>(SDL_LoadFile(compiledFilePath.c_str(), &mSize))),
            mStage(stage) {
            if (mCode == nullptr) {
                throw std::runtime_error("Could not load SPIRV shader from file.");
            }
            SDL_Log("Loaded SPIRV shader from %s of size %zu", compiledFilePath.c_str(), mSize);
        }

        ~ShaderCode() {
            SDL_free(mCode);
            mCode = nullptr;
        }

        ShaderCode(ShaderCode &&other) noexcept :
            mSize(other.mSize), mCode(other.mCode), mStage(other.mStage) {
            other.mCode = nullptr;
        }

        size_t mSize;
        uint8_t *mCode;
        SDL_GPUShaderStage mStage;
    };

    class ResourceLoader {
    public:
        explicit ResourceLoader(sdl::SdlContext const &sdlContext);

        [[nodiscard]] ShaderCode loadShaderCode(std::string const &filename) const;

        [[nodiscard]] shader::MeshData loadObjModel(std::string const &filename) const;

    private:
        sdl::SdlContext mSdlContext;
        std::filesystem::path mProjectRoot = getProjectRoot();
    };
}
