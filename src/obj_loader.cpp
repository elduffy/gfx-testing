#define TINYOBJLOADER_IMPLEMENTATION
#include <filesystem>
#include <format>
#include <iostream>
#include <shader_models.hpp>
#include <tiny_obj_loader.h>
#include <SDL3/SDL_log.h>

namespace gfx_testing::model {
    shader::MeshData loadObjFile(const std::filesystem::path &path) {
        SDL_Log("Loading obj file %s", path.c_str());

        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = path.parent_path();
        reader_config.triangulate = true;
        reader_config.vertex_color = true;

        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(path.string(), reader_config)) {
            if (!reader.Error().empty()) {
                std::cerr << reader.Error() << std::endl;
            }
            throw std::runtime_error("Failed to load obj file");
        }
        if (!reader.Warning().empty()) {
            std::cerr << reader.Warning() << std::endl;
        }

        auto const &attrib = reader.GetAttrib();
        auto const &shapes = reader.GetShapes();
        auto const &materials = reader.GetMaterials();

        SDL_Log("Successfully loaded obj file %s with %zu shapes, %zu materials", path.c_str(), shapes.size(),
                materials.size());

        if (attrib.vertices.size() % 3 != 0) {
            throw std::runtime_error("Vertex count is not a multiple of 3");
        }
        if (attrib.vertices.size() > std::numeric_limits<uint16_t>::max()) {
            throw std::runtime_error("Vertex count is greater than 16 bit limit. Need to refactor now :(");
        }
        if (attrib.vertices.size() != attrib.colors.size()) {
            throw std::runtime_error(std::format("Vertex count {} is not the same as color count {}.",
                                                 attrib.vertices.size(), attrib.colors.size()));
        }
        // if (attrib.vertices.size() != attrib.normals.size()) {
        //     throw std::runtime_error(std::format("Vertex count {} is not the same as normal count {}.",
        //                                          attrib.vertices.size(), attrib.normals.size()));
        // }

        const auto numVertices = attrib.vertices.size() / 3;
        shader::MeshData meshData;

        meshData.vertices.resize(numVertices);
        for (auto i = 0; i < numVertices; i++) {
            auto &[position, normal, color] = meshData.vertices[i];
            position.x = attrib.vertices.at(3 * i);
            position.y = attrib.vertices.at(3 * i + 1);
            position.z = attrib.vertices.at(3 * i + 2);

            // TODO: load the normals
            normal.x = 0;
            normal.y = 0;
            normal.z = 1;

            color.r = attrib.colors.at(3 * i);
            color.g = attrib.colors.at(3 * i + 1);
            color.b = attrib.colors.at(3 * i + 2);
            color.a = 1.f;
        }

        for (auto const &shape: shapes) {
            SDL_Log("Shape %s has %zu indices", shape.name.c_str(), shape.mesh.indices.size());
            meshData.indices.reserve(meshData.indices.size() + shape.mesh.indices.size());

            for (auto const &index: shape.mesh.indices) {
                meshData.indices.push_back(index.vertex_index);
            }
        }

        return meshData;
    }
}
