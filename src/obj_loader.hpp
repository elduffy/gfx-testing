#pragma once

#include <filesystem>
#include <shader_models.hpp>

namespace gfx_testing::model {
    shader::MeshData loadObjFile(std::filesystem::path const &path);
}
