#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <io/gltf_loader.hpp>
#include <io/texture_loader.hpp>
#include <shader/object.hpp>
// ReSharper disable once CppUnusedIncludeDirective
#include <fastgltf/glm_element_traits.hpp>

namespace gfx_testing::io {
    // Arbitrary limit, could probably be increased
    static constexpr auto MAX_COLOR_ATTRIBUTES = 2;

    struct MaterialAttributes {
        size_t mTexcoordIndex;
    };

    util::optref<const fastgltf::Accessor>
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

    sdl::SdlSurface loadBufferViewImage(fastgltf::Asset const &asset,
                                        fastgltf::sources::BufferView const &bufferViewSource) {
        auto const &bufferView = asset.bufferViews.at(bufferViewSource.bufferViewIndex);
        auto const &buffer = asset.buffers.at(bufferView.bufferIndex);
        CHECK(!std::holds_alternative<fastgltf::sources::BufferView>(buffer.data))
                << "Image buffer " << buffer.name << " unexpectedly has BufferView source";
        // SDL_Log("Image buffer '%s' with type %hhu has alternative %lu", buffer.name.c_str(),
        //         static_cast<uint8_t>(bufferViewSource.mimeType), buffer.data.index());
        if (std::holds_alternative<fastgltf::sources::Array>(buffer.data)) {
            auto const &arr = std::get<fastgltf::sources::Array>(buffer.data);
            return loadImage(arr.bytes.data() + bufferView.byteOffset, bufferView.byteLength);
        }

        CHECK(false) << "Image buffer " << buffer.name << " has unsupported alternative " << buffer.data.index();
    }

    sdl::SdlSurface loadImageData(fastgltf::Asset const &asset, fastgltf::Image const &image) {
        // SDL_Log("Image %s has alternative %zu", image.name.c_str(), image.data.index());
        if (std::holds_alternative<fastgltf::sources::BufferView>(image.data)) {
            auto const &bufferView = std::get<fastgltf::sources::BufferView>(image.data);
            return loadBufferViewImage(asset, bufferView);
        }
        CHECK(false) << "Image data alternative not supported: " << image.data.index();
    }

    SDL_GPUSamplerAddressMode getAddressMode(fastgltf::Wrap wrap) {
        switch (wrap) {
            case fastgltf::Wrap::ClampToEdge:
                return SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
            case fastgltf::Wrap::MirroredRepeat:
                return SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT;
            case fastgltf::Wrap::Repeat:
                return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
        }
        throw std::runtime_error("Unrecognized wrap mode");
    }

    SDL_GPUFilter getFilter(fastgltf::Optional<fastgltf::Filter> const &filter) {
        if (!filter.has_value()) {
            return SDL_GPU_FILTER_LINEAR;
        }
        switch (filter.value()) {
            case fastgltf::Filter::Nearest:
            case fastgltf::Filter::NearestMipMapNearest:
            case fastgltf::Filter::LinearMipMapNearest:
                return SDL_GPU_FILTER_NEAREST;
            case fastgltf::Filter::Linear:
            case fastgltf::Filter::NearestMipMapLinear:
            case fastgltf::Filter::LinearMipMapLinear:
                return SDL_GPU_FILTER_LINEAR;
        }
    }

    SDL_GPUSamplerCreateInfo getSamplerCreateInfo(fastgltf::Sampler const &sampler) {
        return SDL_GPUSamplerCreateInfo{
                .min_filter = getFilter(sampler.minFilter),
                .mag_filter = getFilter(sampler.magFilter),
                .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
                .address_mode_u = getAddressMode(sampler.wrapS),
                .address_mode_v = getAddressMode(sampler.wrapT),
                .max_anisotropy = 4,
                .enable_anisotropy = true,
        };
    }

    void addToMesh(util::Mesh &mesh, fastgltf::Asset const &asset, fastgltf::Primitive const &primitive,
                   fastgltf::math::fmat4x4 const &transform,
                   std::optional<MaterialAttributes> const &materialAttributes, std::string_view objectName) {

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

        const auto &normalAccessor = getAccessorOrThrow(asset, primitive, "NORMAL");

        util::optref<const fastgltf::Accessor> texCoordAccessor{};
        if (materialAttributes.has_value()) {
            // Textures referenced in the gltf will use this
            texCoordAccessor.emplace(getAccessorOrThrow(
                    asset, primitive, std::format("TEXCOORD_{}", materialAttributes.value().mTexcoordIndex)));
        } else {
            // If no texture in gltf, still try to look up texcoords so that external textures can be applied.
            texCoordAccessor = getAccessor(asset, primitive, "TEXCOORD_0");
        }

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

            if (texCoordAccessor.has_value()) {
                vertexData.mUv = fastgltf::getAccessorElement<glm::vec2>(asset, texCoordAccessor.value(), index);
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

    std::optional<MaterialAttributes> addMaterial(std::vector<shader::ImageData> &imagesOut,
                                                  fastgltf::Asset const &asset, fastgltf::Primitive const &primitive,
                                                  std::string_view objectName) {
        if (!primitive.materialIndex.has_value()) {
            return std::nullopt;
        }
        auto const &material = asset.materials.at(primitive.materialIndex.value());
        if (!material.pbrData.baseColorTexture.has_value()) {
            return std::nullopt;
        }
        auto const &textureInfo = material.pbrData.baseColorTexture.value();
        CHECK_EQ(textureInfo.transform, nullptr) << "No support for KHR_texture_transform right now";
        auto const &texture = asset.textures.at(textureInfo.textureIndex);
        CHECK(texture.imageIndex.has_value())
                << "No image index for texture " << texture.name << " of mesh " << objectName;

        auto const &image = asset.images.at(texture.imageIndex.value());
        std::vector<sdl::SdlSurface> surfaces;
        surfaces.emplace_back(loadImageData(asset, image));
        auto &imageOut = imagesOut.emplace_back(SDL_GPU_TEXTURETYPE_2D, std::move(surfaces));
        if (texture.samplerIndex.has_value()) {
            imageOut.mSamplerCreateInfo.emplace(getSamplerCreateInfo(asset.samplers.at(texture.samplerIndex.value())));
        }

        return MaterialAttributes{.mTexcoordIndex = textureInfo.texCoordIndex};
    }

    shader::ShaderObject loadGltfFile(const std::filesystem::path &path, util::AttribTreatment attribTreatment) {
        fastgltf::Parser parser;
        auto data = fastgltf::GltfDataBuffer::FromPath(path);
        CHECK(data.error() == fastgltf::Error::None) << "Failed to load GLTF file " << path;

        auto assetResult = parser.loadGltf(data.get(), path.parent_path(), fastgltf::Options::None);
        CHECK(assetResult.error() == fastgltf::Error::None) << "Failed to parse GLTF file " << path;
        auto &asset = assetResult.get();

        CHECK(asset.defaultScene.has_value()) << "No default scene in GLTF file " << path;
        auto const sceneNode = asset.defaultScene.value();
        // SDL_Log("GLTF '%s' using scene %lu: '%s'", path.c_str(), sceneNode, asset.scenes.at(sceneNode).name.c_str());

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

        util::Mesh meshOut;
        std::vector<shader::ImageData> imagesOut;
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
                    util::joinToString(primitive.attributes, ", ", [](auto const &att) { return att.name; }).c_str());

            auto const materialAttributes = addMaterial(imagesOut, asset, primitive, mesh.name);
            addToMesh(meshOut, asset, primitive, transform, materialAttributes, mesh.name);
        }

        return shader::ShaderObject{meshOut.getMeshData(attribTreatment), std::move(imagesOut)};
    }
} // namespace gfx_testing::io
