#define TINYOBJLOADER_IMPLEMENTATION
#include <SDL3/SDL_log.h>
#include <filesystem>
#include <format>
#include <iostream>
#include <shader/shader_models.hpp>
#include <tiny_obj_loader.h>
#include <util/obj_loader.hpp>

namespace gfx_testing::util {

    shader::MeshData processAveraged(tinyobj::ObjReader const &reader) {
        auto const &attrib = reader.GetAttrib();
        auto const &shapes = reader.GetShapes();
        const auto numVertices = attrib.vertices.size() / 3;

        std::vector<std::set<uint16_t>> normalsPerVertex(numVertices);
        shader::MeshDataBuilder meshData;

        meshData.mVertices.resize(numVertices);
        for (auto i = 0; i < numVertices; i++) {
            auto &[position, uv, normal, color] = meshData.mVertices[i];
            position.x = attrib.vertices.at(3 * i);
            position.y = attrib.vertices.at(3 * i + 1);
            position.z = attrib.vertices.at(3 * i + 2);

            uv.x = 0;
            uv.y = 0;

            normal.x = 0;
            normal.y = 0;
            normal.z = 0;

            color.r = attrib.colors.at(3 * i);
            color.g = attrib.colors.at(3 * i + 1);
            color.b = attrib.colors.at(3 * i + 2);
            color.a = 1.f;
        }

        for (auto const &shape: shapes) {
            SDL_Log("Shape '%s' has %zu indices", shape.name.c_str(), shape.mesh.indices.size());
            // meshData.mIndices.reserve(meshData.mIndices.size() + shape.mesh.indices.size());

            for (auto const &index: shape.mesh.indices) {
                assert(index.vertex_index >= 0);
                meshData.addIndex(index.vertex_index);
                if (index.normal_index >= 0) {
                    normalsPerVertex.at(index.vertex_index).emplace(index.normal_index);
                }
            }
        }

        for (auto v = 0; v < numVertices; v++) {
            auto &vertexData = meshData.mVertices.at(v);
            auto const &normalList = normalsPerVertex.at(v);
            vertexData.mNormal = glm::vec3(0);
            for (auto const normalIdx: normalList) {
                vertexData.mNormal += glm::vec3{attrib.normals.at(3 * normalIdx), attrib.normals.at(3 * normalIdx + 1),
                                                attrib.normals.at(3 * normalIdx + 2)};
            }
            vertexData.mNormal /= normalList.size();
            vertexData.mNormal = glm::normalize(vertexData.mNormal);
        }

        // SDL_Log("Mesh data with averaged normals %s", meshData.toString().c_str());
        return meshData.build();
    }

    shader::MeshData processSplit(tinyobj::ObjReader const &reader) {
        auto const &attrib = reader.GetAttrib();
        auto const &shapes = reader.GetShapes();

        // First, map the raw input to their positions/normals
        std::vector<glm::vec3> positions(attrib.vertices.size() / 3);
        std::vector<glm::vec2> uvs(attrib.texcoords.size() / 2);
        std::vector<glm::vec3> normals(attrib.normals.size() / 3);
        std::vector<glm::vec4> colors(attrib.colors.size() / 3);

        for (size_t i = 0; i < positions.size(); i++) {
            positions[i] =
                    glm::vec3(attrib.vertices.at(3 * i), attrib.vertices.at(3 * i + 1), attrib.vertices.at(3 * i + 2));
        }
        for (size_t i = 0; i < uvs.size(); i++) {
            // SDL uses top-to-bottom coordinates, while OBJ assumes bottom-to-top, so the v is inverted.
            uvs[i] = glm::vec2(attrib.texcoords.at(2 * i), 1.f - attrib.texcoords.at(2 * i + 1));
        }
        for (size_t i = 0; i < normals.size(); i++) {
            normals[i] =
                    glm::vec3(attrib.normals.at(3 * i), attrib.normals.at(3 * i + 1), attrib.normals.at(3 * i + 2));
        }
        for (size_t i = 0; i < colors.size(); i++) {
            colors[i] =
                    glm::vec4(attrib.colors.at(3 * i), attrib.colors.at(3 * i + 1), attrib.colors.at(3 * i + 2), 1.f);
        }

        shader::MeshDataBuilder meshData;

        // Loop through the faces to choose output indices for each distinct vertex/normal pair
        // vertex + normal index
        using vn = std::pair<uint16_t, uint16_t>;
        std::map<vn, size_t> outputIndices;
        size_t nextOutputIndex = 0;
        for (auto const &shape: shapes) {
            for (size_t faceIdx = 0; faceIdx < shape.mesh.num_face_vertices.size(); faceIdx++) {
                CHECK_EQ(shape.mesh.num_face_vertices[faceIdx], 3) << "Face vertex count not supported";

                for (size_t vIdx = 0; vIdx < 3; vIdx++) {
                    auto const &index = shape.mesh.indices.at(3 * faceIdx + vIdx);
                    assert(index.vertex_index >= 0);
                    vn vertNormIdx{index.vertex_index, index.normal_index};

                    if (!outputIndices.contains(vertNormIdx)) {
                        outputIndices[vertNormIdx] = nextOutputIndex++;
                        auto &[mPosition, mUv, mNormal, mColor] = meshData.mVertices.emplace_back();
                        mPosition = positions.at(index.vertex_index);
                        if (index.texcoord_index >= 0) {
                            mUv = uvs.at(index.texcoord_index);
                        }
                        if (index.normal_index >= 0) {
                            mNormal = normals.at(index.normal_index);
                        }
                        mColor = colors.at(index.vertex_index);
                    }

                    auto const outputIndex = outputIndices[vertNormIdx];
                    meshData.addIndex(outputIndex);
                }
            }
        }

        // SDL_Log("Mesh data with split normals %s", meshData.toString().c_str());
        return meshData.build();
    }

    shader::MeshData loadObjFile(const std::filesystem::path &path, NormalTreatment normalTreatment) {
        SDL_Log("Loading obj file %s", path.c_str());

        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = path.parent_path();
        reader_config.triangulate = true;
        reader_config.vertex_color = true;

        tinyobj::ObjReader reader;

        CHECK(reader.ParseFromFile(path.string(), reader_config))
                << "Failed to load obj file " << path << ": " << reader.Error();
        if (!reader.Warning().empty()) {
            std::cerr << reader.Warning() << std::endl;
        }

        auto const &attrib = reader.GetAttrib();
        auto const &shapes = reader.GetShapes();
        auto const &materials = reader.GetMaterials();

        SDL_Log("Successfully loaded obj file %s with %zu shapes, %zu materials", path.c_str(), shapes.size(),
                materials.size());

        CHECK_EQ(attrib.vertices.size() % 3, 0) << "Vertex count is not a multiple of 3";
        CHECK_EQ(attrib.vertices.size(), attrib.colors.size())
                << "Vertex count " << attrib.vertices.size() << " is not the same as color count "
                << attrib.colors.size();

        switch (normalTreatment) {
            case NormalTreatment::AVERAGE:
                return processAveraged(reader);
            case NormalTreatment::SPLIT:
                return processSplit(reader);
            default:
                throw std::runtime_error("Unrecognized normal treatment");
        }
    }
} // namespace gfx_testing::util
