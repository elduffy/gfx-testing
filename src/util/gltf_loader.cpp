#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <glm/fwd.hpp>
#include <util/gltf_loader.hpp>

namespace gfx_testing::util {

    std::optional<std::reference_wrapper<const fastgltf::Accessor>>
    getAccessor(fastgltf::Asset const &asset, fastgltf::Primitive const &primitive, char const *attributeName) {
        const auto *iter = primitive.findAttribute(attributeName);
        if (iter == primitive.attributes.end()) {
            return std::nullopt;
        }
        return std::optional{std::ref(asset.accessors[iter->accessorIndex])};
    }

    fastgltf::Accessor const &getAccessorOrThrow(fastgltf::Asset const &asset, fastgltf::Primitive const &primitive,
                                                 char const *attributeName) {
        const auto accessorOpt = getAccessor(asset, primitive, attributeName);
        CHECK(accessorOpt.has_value()) << "Missing attribute " << attributeName;
        return accessorOpt.value().get();
    }

    shader::MeshData loadGltfFile(const std::filesystem::path &path, AttribTreatment attribTreatment) {
        fastgltf::Parser parser;
        auto data = fastgltf::GltfDataBuffer::FromPath(path);
        CHECK(data.error() == fastgltf::Error::None) << "Failed to load GLTF file " << path;

        auto asset = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::None);
        CHECK(asset.error() == fastgltf::Error::None) << "Failed to parse GLTF file " << path;

        fastgltf::Primitive *primitive = nullptr;
        for (auto &mesh: asset->meshes) {
            CHECK_EQ(mesh.primitives.size(), 1)
                    << "Expected 1 primitive for mesh " << mesh.name << ", got " << mesh.primitives.size();
            primitive = &mesh.primitives.front();
            CHECK(primitive->type == fastgltf::PrimitiveType::Triangles)
                    << "Unsupported primitive type for mesh " << mesh.name << ": "
                    << static_cast<uint8_t>(primitive->type);
            CHECK(primitive->indicesAccessor.has_value()) << "Mesh " << mesh.name << " has no indices accessor";

            SDL_Log("Mesh '%s' has attributes %s", mesh.name.c_str(),
                    joinToString(primitive->attributes, ", ", [](auto const &att) { return att.name; }).c_str());
        }
        CHECK_NE(primitive, nullptr) << "No primitives found in gltf file " << path;

        const auto &positionAccessor = getAccessorOrThrow(asset.get(), *primitive, "POSITION");
        const auto &colorAccessorOpt = getAccessor(asset.get(), *primitive, "COLOR_0");
        const auto &uvAccessorOpt = getAccessor(asset.get(), *primitive, "TEXCOORD_0");
        const auto &normalAccessor = getAccessorOrThrow(asset.get(), *primitive, "NORMAL");

        Mesh mesh;

        std::vector<shader::VertexData> triangleBuffer;
        triangleBuffer.reserve(3);

        auto const &indexAccessor = asset->accessors[primitive->indicesAccessor.value()];
        fastgltf::iterateAccessor<uint32_t>(asset.get(), indexAccessor, [&](auto const &index) {
            auto const position =
                    fastgltf::getAccessorElement<fastgltf::math::fvec3>(asset.get(), positionAccessor, index);
            auto const normal = fastgltf::getAccessorElement<fastgltf::math::fvec3>(asset.get(), normalAccessor, index);
            auto &vertexData = triangleBuffer.emplace_back();
            vertexData.mPosition = {position.x(), position.y(), position.z()};
            vertexData.mNormal = {normal.x(), normal.y(), normal.z()};
            if (uvAccessorOpt.has_value()) {
                auto const uv =
                        fastgltf::getAccessorElement<fastgltf::math::fvec2>(asset.get(), uvAccessorOpt.value(), index);
                vertexData.mUv = {uv.x(), uv.y()};
            } else {
                vertexData.mUv = glm::vec2(0);
            }
            if (colorAccessorOpt.has_value()) {
                auto const color = fastgltf::getAccessorElement<fastgltf::math::fvec4>(asset.get(),
                                                                                       colorAccessorOpt.value(), index);
                vertexData.mColor = {color.x(), color.y(), color.z(), color.w()};
            } else {
                vertexData.mColor = glm::vec4(1);
            }

            if (triangleBuffer.size() == 3) {
                auto const &triangle = mesh.addTriangle(triangleBuffer[0].mPosition, triangleBuffer[1].mPosition,
                                                        triangleBuffer[2].mPosition);
                for (int i = 0; i < 3; ++i) {
                    mesh.setVertexNormal(triangle.mVertexIndices[i], triangleBuffer[i].mNormal);
                    mesh.setColor(triangle.mVertexIndices[i], triangleBuffer[i].mColor);
                    mesh.setUv(triangle.mVertexIndices[i], triangleBuffer[i].mUv);
                }
                triangleBuffer.clear();
            }
        });

        CHECK(triangleBuffer.empty()) << "Vertices left over processing file " << path;


        return mesh.getMeshData(attribTreatment);
    }
} // namespace gfx_testing::util
