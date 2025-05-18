#pragma once
#include <filesystem>
#include <shader/shader_models.hpp>
#include <util/mesh.hpp>

namespace gfx_testing::util {
    shader::MeshData loadGltfFile(const std::filesystem::path &path, AttribTreatment attribTreatment);
}
