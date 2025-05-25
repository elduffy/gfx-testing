#pragma once
#include <filesystem>
#include <game.hpp>
#include <shader/object.hpp>
#include <util/mesh.hpp>

namespace gfx_testing::io {
    shader::ShaderObject loadGltfFile(const std::filesystem::path &path, util::AttribTreatment attribTreatment);
}
