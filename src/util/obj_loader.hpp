#pragma once

#include <filesystem>
#include <shader/shader_models.hpp>

namespace gfx_testing::util {

    enum class NormalTreatment {
        /**
         * Keeps the same number of vertices and averages the normals for each face together.
         * Not suitable for texture mapping.
         */
        AVERAGE,
        /**
         * Creates new vertices as needed to hold normals/UVs for all faces
         */
        SPLIT,
    };

    shader::MeshData loadObjFile(std::filesystem::path const &path, NormalTreatment normalTreatment);
} // namespace gfx_testing::util
