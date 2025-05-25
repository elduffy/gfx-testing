#include <boost/algorithm/string/replace.hpp>
#include <io/gltf_loader.hpp>
#include <io/obj_loader.hpp>
#include <io/resource_loader.hpp>
#include <io/texture_loader.hpp>

namespace gfx_testing::io {

    ResourceLoader::ResourceLoader(sdl::SdlContext const &sdlContext) : mSdlContext(sdlContext) {}

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

        CHECK(SDL_GetGPUShaderFormats(mSdlContext.mDevice) & SDL_GPU_SHADERFORMAT_SPIRV)
                << "Only supporting SPIRV formats for now";

        std::string compiledFilename = shaderSourcePath.filename().string();
        boost::replace_last(compiledFilename, ".hlsl", ".spv");
        const auto compiledFilePath =
                shaderSourcePath.parent_path().parent_path() / "compiled/spirv" / compiledFilename;
        return ShaderCode(compiledFilePath, stage);
    }

    shader::ShaderObject ResourceLoader::loadObjModel(std::string const &filename,
                                                      util::AttribTreatment attribTreatment) const {
        return loadObjFile(mProjectRoot / "content/models/" / filename, attribTreatment);
    }

    shader::ShaderObject ResourceLoader::loadGltfModel(std::string const &filename,
                                                       util::AttribTreatment attribTreatment) const {
        return loadGltfFile(mProjectRoot / "content/models/" / filename, attribTreatment);
    }

    sdl::SdlSurface ResourceLoader::loadSurface(std::string const &filename) const {
        return loadImage(mProjectRoot / "content/textures/" / filename);
    }

    util::CubeMap ResourceLoader::loadCubeMap(std::string const &dirname) const {
        return io::loadCubeMap(mProjectRoot / "content/cubemaps/" / dirname);
    }
} // namespace gfx_testing::io
