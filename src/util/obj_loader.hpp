#pragma once

#include <filesystem>
#include <shader/object.hpp>
#include <util/mesh.hpp>

namespace gfx_testing::util {

    shader::ShaderObject loadObjFile(std::filesystem::path const &path, AttribTreatment attribTreatment);
} // namespace gfx_testing::util
