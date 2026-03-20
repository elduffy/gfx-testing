#include <boost/algorithm/string/replace.hpp>
#include <io/gltf_loader.hpp>
#include <io/obj_loader.hpp>
#include <io/resource_loader.hpp>
#include <io/texture_loader.hpp>
#include <util/debug.hpp>

namespace gfx_testing::io {
    std::filesystem::path getShaderDir(std::filesystem::path const &root) {
        // TODO: this is hacky, but it works for the time being. Would be better to get an arg or env for this.
        return root / (util::DEBUG_MODE ? "cmake-build-debug" : "cmake-build-release") / "shaders/compiled/spirv";
    }

    ResourceLoader::ResourceLoader(sdl::SdlContext const &sdlContext) : mSdlContext(sdlContext) {}

    ShaderCode ResourceLoader::loadShaderCode(std::string const &filename) const {
        auto const shaderSourcePath = mProjectRoot / "content/shaders/src" / filename;
        shader::ShaderType type;
        if (SDL_strstr(shaderSourcePath.filename().c_str(), ".vert")) {
            type = shader::ShaderType::Vertex;
        } else if (SDL_strstr(shaderSourcePath.filename().c_str(), ".frag")) {
            type = shader::ShaderType::Fragment;
        } else if (SDL_strstr(shaderSourcePath.filename().c_str(), ".comp")) {
            type = shader::ShaderType::Compute;
        } else {
            FAIL("Could not determine shader type for file '{}'", filename);
        }

        CHECK(SDL_GetGPUShaderFormats(mSdlContext.mDevice) & SDL_GPU_SHADERFORMAT_SPIRV)
                << "Only supporting SPIRV formats for now";

        std::string compiledFilename = shaderSourcePath.filename().string();
        boost::replace_last(compiledFilename, ".hlsl", ".spv");
        const auto compiledFilePath = getShaderDir(mProjectRoot) / compiledFilename;
        return ShaderCode(compiledFilePath, type);
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
    SceneDefinition ResourceLoader::loadSceneDefinition(std::string const &filename) const {
        return io::loadSceneDefinition(mProjectRoot / "content/scenes" / filename);
    }
} // namespace gfx_testing::io
