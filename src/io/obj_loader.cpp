#define TINYOBJLOADER_IMPLEMENTATION
#include <SDL3/SDL_log.h>
#include <filesystem>
#include <format>
#include <io/obj_loader.hpp>
#include <iostream>
#include <shader/shader_models.hpp>
#include <tiny_obj_loader.h>

namespace gfx_testing::io {

    void addToMesh(tinyobj::ObjReader const &reader, util::Mesh &mesh) {
        auto const &attrib = reader.GetAttrib();

        for (auto const &shapes = reader.GetShapes(); auto const &shape: shapes) {
            for (size_t faceIdx = 0; faceIdx < shape.mesh.num_face_vertices.size(); faceIdx++) {
                CHECK_EQ(shape.mesh.num_face_vertices[faceIdx], 3) << "Face vertex count not supported";

                std::array<glm::vec3, 3> triPos;
                std::array<glm::vec4, 3> triCol;
                std::array<glm::vec3, 3> triNorm;
                std::array<glm::vec2, 3> triUv;

                for (size_t vIdx = 0; vIdx < 3; vIdx++) {
                    auto const &index = shape.mesh.indices.at(3 * faceIdx + vIdx);
                    CHECK_GE(index.vertex_index, 0);

                    triPos[vIdx].x = attrib.vertices.at(3 * index.vertex_index);
                    triPos[vIdx].y = attrib.vertices.at(3 * index.vertex_index + 1);
                    triPos[vIdx].z = attrib.vertices.at(3 * index.vertex_index + 2);

                    triCol[vIdx].r = attrib.colors.at(3 * index.vertex_index);
                    triCol[vIdx].g = attrib.colors.at(3 * index.vertex_index + 1);
                    triCol[vIdx].b = attrib.colors.at(3 * index.vertex_index + 2);
                    triCol[vIdx].a = 1;

                    if (index.texcoord_index >= 0) {
                        triUv[vIdx].x = attrib.texcoords.at(2 * index.texcoord_index);
                        triUv[vIdx].y = attrib.texcoords.at(2 * index.texcoord_index + 1);
                    } else {
                        triUv[vIdx] = glm::vec2(0.f);
                    }
                    if (index.normal_index >= 0) {
                        triNorm[vIdx].x = attrib.normals.at(3 * index.normal_index);
                        triNorm[vIdx].y = attrib.normals.at(3 * index.normal_index + 1);
                        triNorm[vIdx].z = attrib.normals.at(3 * index.normal_index + 2);
                    } else {
                        triNorm[vIdx] = glm::vec3(0);
                    }
                }

                auto const &triangle = mesh.addTriangle(triPos[0], triPos[1], triPos[2]);
                for (size_t vIdx = 0; vIdx < 3; vIdx++) {
                    auto const index = triangle.mVertexIndices[vIdx];
                    mesh.setColor(index, triCol[vIdx]);
                    mesh.setUv(index, triUv[vIdx]);
                    mesh.setVertexNormal(index, triNorm[vIdx]);
                }
            }
        }
    }

    shader::ShaderObject loadObjFile(const std::filesystem::path &path, util::AttribTreatment attribTreatment) {
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

        util::Mesh mesh;
        addToMesh(reader, mesh);
        return shader::ShaderObject{mesh.getMeshData(attribTreatment), {}};
    }
} // namespace gfx_testing::io
