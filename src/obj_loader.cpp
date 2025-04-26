#define TINYOBJLOADER_IMPLEMENTATION
#include <filesystem>
#include <format>
#include <iostream>
#include <numeric>
#include <shader_models.hpp>
#include <tiny_obj_loader.h>
#include <SDL3/SDL_log.h>
#include <obj_loader.hpp>

namespace gfx_testing::model {


    shader::VertexData mergeVertexData(std::vector<shader::VertexData> const &vertexData) {
        shader::VertexData merged{
                .mPosition = vertexData[0].mPosition,
        };
        for (const auto &[mPosition, mNormal, mColor]: vertexData) {
            assert(mPosition == merged.mPosition);
            merged.mNormal += mNormal;
            merged.mColor += mColor;
        }
        merged.mNormal /= vertexData.size();
        merged.mNormal = glm::normalize(merged.mNormal);
        merged.mColor /= vertexData.size();
        return merged;
    }

    shader::MeshData processAveraged(tinyobj::ObjReader const &reader) {
        auto const &attrib = reader.GetAttrib();
        auto const &shapes = reader.GetShapes();
        const auto numVertices = attrib.vertices.size() / 3;

        std::vector<std::set<uint16_t> > normalsPerVertex(numVertices);
        shader::MeshData meshData;

        meshData.mVertices.resize(numVertices);
        for (auto i = 0; i < numVertices; i++) {
            auto &[position, normal, color] = meshData.mVertices[i];
            position.x = attrib.vertices.at(3 * i);
            position.y = attrib.vertices.at(3 * i + 1);
            position.z = attrib.vertices.at(3 * i + 2);

            normal.x = 0;
            normal.y = 0;
            normal.z = 0;

            color.r = attrib.colors.at(3 * i);
            color.g = attrib.colors.at(3 * i + 1);
            color.b = attrib.colors.at(3 * i + 2);
            color.a = 1.f;
        }

        for (auto const &shape: shapes) {
            SDL_Log("Shape %s has %zu indices", shape.name.c_str(), shape.mesh.indices.size());
            meshData.mIndices.reserve(meshData.mIndices.size() + shape.mesh.indices.size());

            for (auto const &index: shape.mesh.indices) {
                meshData.mIndices.push_back(index.vertex_index);
                normalsPerVertex.at(index.vertex_index).emplace(index.normal_index);
            }
        }

        for (auto v = 0; v < numVertices; v++) {
            auto &vertexData = meshData.mVertices.at(v);
            auto const &normalList = normalsPerVertex.at(v);
            vertexData.mNormal = glm::vec3(0);
            for (auto const normalIdx: normalList) {
                vertexData.mNormal += glm::vec3{
                        attrib.normals.at(3 * normalIdx),
                        attrib.normals.at(3 * normalIdx + 1),
                        attrib.normals.at(3 * normalIdx + 2)
                };
            }
            vertexData.mNormal /= normalList.size();
            vertexData.mNormal = glm::normalize(vertexData.mNormal);
        }

        // SDL_Log("Mesh data with averaged normals %s", meshData.toString().c_str());
        return meshData;
    }

    shader::MeshData processSplit(tinyobj::ObjReader const &reader) {
        auto const &attrib = reader.GetAttrib();
        auto const &shapes = reader.GetShapes();

        // First, map the raw input to their positions/normals
        std::vector<glm::vec3> positions(attrib.vertices.size() / 3);
        std::vector<glm::vec3> normals(attrib.normals.size() / 3);
        std::vector<glm::vec4> colors(attrib.colors.size() / 3);

        for (size_t i = 0; i < positions.size(); i++) {
            positions[i] = glm::vec3(attrib.vertices.at(3 * i),
                                     attrib.vertices.at(3 * i + 1),
                                     attrib.vertices.at(3 * i + 2));
        }
        for (size_t i = 0; i < normals.size(); i++) {
            normals[i] = glm::vec3(attrib.normals.at(3 * i),
                                   attrib.normals.at(3 * i + 1),
                                   attrib.normals.at(3 * i + 2));
        }
        for (size_t i = 0; i < colors.size(); i++) {
            colors[i] = glm::vec4(attrib.colors.at(3 * i),
                                  attrib.colors.at(3 * i + 1),
                                  attrib.colors.at(3 * i + 2),
                                  1.f);
        }

        shader::MeshData meshData;

        // Loop through the faces to choose output indices for each distinct vertex/normal pair
        // vertex + normal index
        using vn = std::pair<uint16_t, uint16_t>;
        std::map<vn, size_t> outputIndices;
        size_t nextOutputIndex = 0;
        for (auto const &shape: shapes) {
            for (size_t faceIdx = 0; faceIdx < shape.mesh.num_face_vertices.size(); faceIdx++) {
                if (shape.mesh.num_face_vertices[faceIdx] != 3) {
                    throw std::runtime_error("Face vertex count not supported");
                }

                for (size_t vIdx = 0; vIdx < 3; vIdx++) {
                    auto const &index = shape.mesh.indices.at(3 * faceIdx + vIdx);
                    vn vertNormIdx{index.vertex_index, index.normal_index};

                    if (!outputIndices.contains(vertNormIdx)) {
                        outputIndices[vertNormIdx] = nextOutputIndex++;
                        auto &[mPosition, mNormal, mColor] = meshData.mVertices.emplace_back();
                        mPosition = positions.at(index.vertex_index);
                        mNormal = normals.at(index.normal_index);
                        mColor = colors.at(index.vertex_index);
                    }

                    auto const outputIndex = outputIndices[vertNormIdx];
                    meshData.mIndices.push_back(outputIndex);
                }

            }
        }

        // SDL_Log("Mesh data with split normals %s", meshData.toString().c_str());
        return meshData;
    }

    shader::MeshData loadObjFile(const std::filesystem::path &path, NormalTreatment normalTreatment) {
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


        switch (normalTreatment) {
            case NormalTreatment::AVERAGE:
                return processAveraged(reader);
            case NormalTreatment::SPLIT:
                return processSplit(reader);
            default:
                throw std::runtime_error("Unrecognized normal treatment");
        }
    }
}
