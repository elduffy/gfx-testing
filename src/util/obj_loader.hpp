#pragma once

#include <filesystem>
#include <shader/shader_models.hpp>
#include <util/mesh.hpp>

namespace gfx_testing::util {

    shader::MeshData loadObjFile(std::filesystem::path const &path, AttribTreatment attribTreatment);
} // namespace gfx_testing::util
