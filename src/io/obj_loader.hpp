#pragma once

#include <filesystem>

#include <shader/object.hpp>
#include <util/mesh.hpp>

namespace gfx_testing::io {

    shader::ShaderObject loadObjFile(std::filesystem::path const &path, util::AttribTreatment attribTreatment);
} // namespace gfx_testing::io
