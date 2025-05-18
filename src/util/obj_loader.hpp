#pragma once

#include <filesystem>
#include <shader/shader_models.hpp>
#include <util/mesh.hpp>

namespace gfx_testing::util {

    shader::MeshData loadObjFile(std::filesystem::path const &path, NormalTreatment normalTreatment,
                                 TexCoordTreatment texCoordTreatment);
} // namespace gfx_testing::util
