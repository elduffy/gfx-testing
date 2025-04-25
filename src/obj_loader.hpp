#pragma once

#include <filesystem>
#include <shader_models.hpp>

namespace gfx_testing::model {

    enum class NormalTreatment {
        /**
         * Keeps the same number of vertices and averages the normals for each face together
         */
        AVERAGE,
        /**
         * Creates new vertices as needed to hold normals for all faces
         */
        SPLIT,
    };

    shader::MeshData loadObjFile(std::filesystem::path const &path, NormalTreatment normalTreatment);
}
