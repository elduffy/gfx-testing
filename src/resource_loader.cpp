#include <obj_loader.hpp>
#include <resource_loader.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "texture.hpp"

namespace gfx_testing::util {

    ResourceLoader::ResourceLoader(sdl::SdlContext const &sdlContext):
        mSdlContext(sdlContext) {
    }

    ShaderCode ResourceLoader::loadShaderCode(std::string const &filename) const {
        auto const shaderSourcePath = mProjectRoot / "content/shaders/src" / filename;
        SDL_GPUShaderStage stage;
        if (SDL_strstr(shaderSourcePath.filename().c_str(), ".vert")) {
            stage = SDL_GPU_SHADERSTAGE_VERTEX;
        } else if (SDL_strstr(shaderSourcePath.filename().c_str(), ".frag")) {
            stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
        } else {
            throw std::runtime_error("Could not determine shader stage for file.");
        }

        if (!(SDL_GetGPUShaderFormats(mSdlContext.mDevice) & SDL_GPU_SHADERFORMAT_SPIRV)) {
            throw std::runtime_error("Only supporting SPIRV formats for now");
        }

        std::string compiledFilename = shaderSourcePath.filename().string();
        boost::replace_last(compiledFilename, ".hlsl", ".spv");
        const auto compiledFilePath = shaderSourcePath.parent_path().parent_path() / "compiled/spirv" /
                                      compiledFilename;
        return ShaderCode(compiledFilePath, stage);
    }

    shader::MeshData ResourceLoader::loadObjModel(std::string const &filename,
                                                  model::NormalTreatment normalTreatment) const {
        return model::loadObjFile(mProjectRoot / "content/models/" / filename, normalTreatment);
    }

    sdl::SdlSurface ResourceLoader::loadTexture(std::string const &filename) const {
        // TODO: support other formats
        if (filename.ends_with(".bmp")) {
            return io::loadBmp(mProjectRoot / "content/textures/" / filename);
        }
        throw std::runtime_error("Could not determine texture type.");
    }
}
