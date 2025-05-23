#pragma once
#include <filesystem>
#include <shader/object.hpp>
#include <util/mesh.hpp>

namespace gfx_testing::util {
    shader::ShaderObject loadGltfFile(const std::filesystem::path &path, AttribTreatment attribTreatment);
}
