#pragma once

#include <SDL3/SDL.h>
#include <filesystem>
#include <sdl.hpp>
#include <shader/object.hpp>
#include <util/cube_map.hpp>
#include <util/mesh.hpp>
#include <util/util.hpp>

namespace gfx_testing::util {

    class ShaderCode {
    public:
        NO_COPY(ShaderCode);

        explicit ShaderCode(std::filesystem::path const &compiledFilePath, SDL_GPUShaderStage stage) :
            mSize(0), mCode(static_cast<uint8_t *>(SDL_LoadFile(compiledFilePath.c_str(), &mSize))), mStage(stage) {
            CHECK_NE(mCode, nullptr) << "Could not load SPIRV shader from " << compiledFilePath.c_str() << ": "
                                     << SDL_GetError();

            SDL_Log("Loaded SPIRV shader from %s of size %zu", compiledFilePath.c_str(), mSize);
        }

        ~ShaderCode() {
            SDL_free(mCode);
            mCode = nullptr;
        }

        ShaderCode(ShaderCode &&other) noexcept : mSize(other.mSize), mCode(other.mCode), mStage(other.mStage) {
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

        [[nodiscard]] shader::ShaderObject loadObjModel(std::string const &filename,
                                                        AttribTreatment attribTreatment = {}) const;

        [[nodiscard]] shader::ShaderObject loadGltfModel(std::string const &filename,
                                                         AttribTreatment attribTreatment = {}) const;

        [[nodiscard]] sdl::SdlSurface loadSurface(std::string const &filename) const;

        [[nodiscard]] CubeMap loadCubeMap(std::string const &dirname) const;

    private:
        sdl::SdlContext const &mSdlContext;
        std::filesystem::path mProjectRoot = getProjectRoot();
    };
} // namespace gfx_testing::util
