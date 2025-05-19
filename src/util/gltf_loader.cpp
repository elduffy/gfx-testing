#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <util/gltf_loader.hpp>

namespace gfx_testing::util {
    // Arbitrary limit, could probably be increased
    static constexpr auto MAX_COLOR_ATTRIBUTES = 2;

    std::optional<std::reference_wrapper<const fastgltf::Accessor>>
    getAccessor(fastgltf::Asset const &asset, fastgltf::Primitive const &primitive, std::string_view attributeName) {
        const auto *iter = primitive.findAttribute(attributeName);
        if (iter == primitive.attributes.end()) {
            return std::nullopt;
        }
        return std::optional{std::ref(asset.accessors[iter->accessorIndex])};
    }

    fastgltf::Accessor const &getAccessorOrThrow(fastgltf::Asset const &asset, fastgltf::Primitive const &primitive,
                                                 std::string_view attributeName) {
        const auto accessorOpt = getAccessor(asset, primitive, attributeName);
        CHECK(accessorOpt.has_value()) << "Missing attribute " << attributeName;
        return accessorOpt.value().get();
    }

    fastgltf::math::fvec4 extend(fastgltf::math::fvec3 const &v, float w) {
        fastgltf::math::fvec4 result(v);
        result[3] = w;
        return result;
    }

    void processPrimitive(Mesh &mesh, fastgltf::Asset const &asset, fastgltf::Primitive const &primitive,
                          fastgltf::math::fmat4x4 const &transform, std::string_view objectName) {

        const auto &positionAccessor = getAccessorOrThrow(asset, primitive, "POSITION");

        std::vector<std::reference_wrapper<const fastgltf::Accessor>> colorAccessors;
        for (size_t i = 0; i < MAX_COLOR_ATTRIBUTES; i++) {
            const auto colorAccessorOpt = getAccessor(asset, primitive, std::format("COLOR_{}", i));
            if (colorAccessorOpt.has_value()) {
                colorAccessors.push_back(colorAccessorOpt.value());
            } else {
                break;
            }
        }

        const auto uvAccessorOpt = getAccessor(asset, primitive, "TEXCOORD_0");
        const auto &normalAccessor = getAccessorOrThrow(asset, primitive, "NORMAL");

        std::vector<shader::VertexData> triangleBuffer;
        triangleBuffer.reserve(3);

        auto const &indexAccessor = asset.accessors[primitive.indicesAccessor.value()];
        fastgltf::iterateAccessor<uint32_t>(asset, indexAccessor, [&](auto const &index) {
            auto &vertexData = triangleBuffer.emplace_back();
            auto const fvecPos =
                    extend(fastgltf::getAccessorElement<fastgltf::math::fvec3>(asset, positionAccessor, index), 1.f);
            auto const fvecNorm =
                    extend(fastgltf::getAccessorElement<fastgltf::math::fvec3>(asset, normalAccessor, index), 0.f);
            auto const transformedPos = transform * fvecPos;
            auto const transformedNorm = transform * fvecNorm;

            vertexData.mPosition = {transformedPos.x(), transformedPos.y(), transformedPos.z()};
            vertexData.mNormal =
                    glm::normalize(glm::vec3{transformedNorm.x(), transformedNorm.y(), transformedNorm.z()});

            if (uvAccessorOpt.has_value()) {
                vertexData.mUv = fastgltf::getAccessorElement<glm::vec2>(asset, uvAccessorOpt.value(), index);
            } else {
                vertexData.mUv = glm::vec2(0);
            }
            vertexData.mColor = glm::vec4(1);
            for (auto const colorAccessor: colorAccessors) {
                auto const color = glm::clamp(
                        fastgltf::getAccessorElement<glm::vec4>(asset, colorAccessor.get(), index), 0.f, 1.f);
                vertexData.mColor *= color;
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

        CHECK(triangleBuffer.empty()) << "Vertices left over processing " << objectName;
    }

    shader::MeshData loadGltfFile(const std::filesystem::path &path, AttribTreatment attribTreatment) {
        fastgltf::Parser parser;
        auto data = fastgltf::GltfDataBuffer::FromPath(path);
        CHECK(data.error() == fastgltf::Error::None) << "Failed to load GLTF file " << path;

        auto assetResult = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::None);
        CHECK(assetResult.error() == fastgltf::Error::None) << "Failed to parse GLTF file " << path;
        auto &asset = assetResult.get();

        CHECK(asset.defaultScene.has_value()) << "No default scene in GLTF file " << path;
        auto const sceneNode = asset.defaultScene.value();
        SDL_Log("GLTF '%s' using scene %lu: '%s'", path.c_str(), sceneNode, asset.scenes.at(sceneNode).name.c_str());

        // Get the meshes to process along with their transforms
        using mesh_index_t = std::pair<size_t, fastgltf::math::fmat4x4>;
        std::vector<mesh_index_t> meshIndices;
        fastgltf::sceneIndex(asset, sceneNode, fastgltf::math::fmat4x4(),
                             [&](fastgltf::Node const &node, auto const &matrix) {
                                 if (!node.meshIndex.has_value()) {
                                     SDL_Log("Skipping node '%s': no mesh", node.name.c_str());
                                     return;
                                 }
                                 meshIndices.emplace_back(node.meshIndex.value(), matrix);
                             });

        Mesh meshOut;
        for (auto const &[meshIndex, transform]: meshIndices) {
            auto const &mesh = asset.meshes[meshIndex];
            CHECK_EQ(mesh.primitives.size(), 1)
                    << "Expected 1 primitive for mesh " << mesh.name << ", got " << mesh.primitives.size();

            auto const &primitive = mesh.primitives.front();
            CHECK(primitive.type == fastgltf::PrimitiveType::Triangles)
                    << "Unsupported primitive type for mesh " << mesh.name << ": "
                    << static_cast<uint8_t>(primitive.type);
            CHECK(primitive.indicesAccessor.has_value()) << "Mesh " << mesh.name << " has no indices accessor";

            SDL_Log("Mesh '%s' has attributes %s", mesh.name.c_str(),
                    joinToString(primitive.attributes, ", ", [](auto const &att) { return att.name; }).c_str());

            processPrimitive(meshOut, asset, primitive, transform, mesh.name);
        }

        return meshOut.getMeshData(attribTreatment);
    }
} // namespace gfx_testing::util
