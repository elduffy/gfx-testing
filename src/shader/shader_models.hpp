#pragma once
#include <SDL3/SDL.h>
#include <array>
#include <boost/safe_numerics/checked_default.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gooch.frag.hpp>
#include <vector>

#define SHADER_ALIGN alignas(16)

namespace gfx_testing::shader {
    struct VertexData {
        glm::vec3 mPosition;
        glm::vec2 mUv;
        glm::vec3 mNormal;
        glm::vec4 mColor;

        [[nodiscard]] std::string toString() const {
            std::stringstream ss;
            ss << "pos = {" << mPosition.x << ", " << mPosition.y << ", " << mPosition.z << "}, ";
            ss << "uv = {" << mUv.x << ", " << mUv.y << "}, ";
            ss << "norm = {" << mNormal.x << ", " << mNormal.y << ", " << mNormal.z << "}, ";
            ss << "col = {" << mColor.x << ", " << mColor.y << ", " << mColor.z << "}";
            return ss.str();
        }

        static constexpr std::array VERTEX_ATTRIBUTES{
                SDL_GPUVertexAttribute{
                        .location = 0,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                        .offset = 0,
                },
                SDL_GPUVertexAttribute{
                        .location = 1,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                        .offset = sizeof(mPosition),
                },
                SDL_GPUVertexAttribute{
                        .location = 2,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                        .offset = sizeof(mPosition) + sizeof(mUv),
                },
                SDL_GPUVertexAttribute{
                        .location = 3,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                        .offset = sizeof(mPosition) + sizeof(mUv) + sizeof(mNormal),
                }};
    };

    struct MvpTransform {
        glm::mat4 mMvp;
    };

    static_assert(sizeof(MvpTransform) % 16 == 0);

    struct GoochParams {
        SHADER_ALIGN glm::vec3 mCoolColor;
        SHADER_ALIGN glm::vec3 mWarmColor;
    };

    static_assert(sizeof(GoochParams) % 16 == 0);

    struct LambertParams {
        SHADER_ALIGN glm::vec3 mUnlitColor;
        SHADER_ALIGN glm::vec3 mLitColor;
    };

    static_assert(sizeof(LambertParams) % 16 == 0);

    struct vec3_16 {
        SHADER_ALIGN glm::vec3 v;
    };
    static_assert(sizeof(vec3_16) == 16);
    static_assert(offsetof(vec3_16, v) == 0);

    struct ObjectLighting {
        static constexpr size_t MAX_NUM_LIGHTS = 8;
        SHADER_ALIGN glm::vec3 mCameraPosMS;
        SHADER_ALIGN uint32_t mNumLights;
        SHADER_ALIGN std::array<vec3_16, MAX_NUM_LIGHTS> mLightPosMS;

        static ObjectLighting create(glm::mat4 const &modelMatrix, std::vector<glm::vec3> const &lightPosWs,
                                     glm::vec3 const &cameraPosWs) {
            if (lightPosWs.size() > MAX_NUM_LIGHTS) {
                throw std::runtime_error("Too many lights");
            }
            auto const worldToModelTransform = glm::inverse(modelMatrix);
            ObjectLighting lighting{
                    .mNumLights = boost::safe_numerics::checked::cast<uint32_t>(lightPosWs.size()),
                    .mCameraPosMS = worldToModelTransform * glm::vec4(cameraPosWs, 1),
            };
            for (size_t i = 0; i < lightPosWs.size(); ++i) {
                lighting.mLightPosMS[i].v = worldToModelTransform * glm::vec4(lightPosWs.at(i), 1);
            }
            return lighting;
        }
    };

    static_assert(spirv_header_gen::generated::gooch_frag::TYPE_ObjectLighting.mMembers[0].mOffset ==
                  offsetof(ObjectLighting, mCameraPosMS));
    static_assert(spirv_header_gen::generated::gooch_frag::TYPE_ObjectLighting.mMembers[2].mOffset ==
                  offsetof(ObjectLighting, mNumLights));
    static_assert(spirv_header_gen::generated::gooch_frag::TYPE_ObjectLighting.mMembers[3].mOffset ==
                  offsetof(ObjectLighting, mLightPosMS));

    class IndexList {
        template<typename index_t>
        std::unique_ptr<uint8_t> copyVector(std::vector<index_t> const &indices) {
            std::unique_ptr<uint8_t> data(new uint8_t[indices.size() * sizeof(index_t)]);
            index_t *dest = reinterpret_cast<index_t *>(data.get());
            std::copy(indices.begin(), indices.end(), dest);
            return data;
        }

    public:
        explicit IndexList(std::vector<uint16_t> const &indices) :
            mCount(indices.size()), mBuffer(copyVector(indices)), mElementSize(SDL_GPU_INDEXELEMENTSIZE_16BIT) {}

        explicit IndexList(std::vector<uint32_t> const &indices) :
            mCount(indices.size()), mBuffer(copyVector(indices)), mElementSize(SDL_GPU_INDEXELEMENTSIZE_32BIT) {}

        size_t count() const { return mCount; }

        template<typename index_t>
        index_t const *as() const;

        uint16_t const *asUint16() const {
            if (mElementSize != SDL_GPU_INDEXELEMENTSIZE_16BIT) {
                throw std::runtime_error("Attempt to access 32-bit indices as 16-bit");
            }
            return reinterpret_cast<uint16_t const *>(mBuffer.get());
        }

        uint32_t const *asUint32() const {
            if (mElementSize != SDL_GPU_INDEXELEMENTSIZE_32BIT) {
                throw std::runtime_error("Attempt to access 16-bit indices as 32-bit");
            }
            return reinterpret_cast<uint32_t const *>(mBuffer.get());
        }

        size_t elementSize() const {
            switch (mElementSize) {
                case SDL_GPU_INDEXELEMENTSIZE_16BIT:
                    return sizeof(uint16_t);
                case SDL_GPU_INDEXELEMENTSIZE_32BIT:
                    return sizeof(uint32_t);
                default:
                    throw std::runtime_error("Unexpected element size");
            }
        }

        size_t bufferSize() const { return count() * elementSize(); }

        std::string toString() const {
            std::stringstream ss;
            ss << "[";
            switch (mElementSize) {
                case SDL_GPU_INDEXELEMENTSIZE_16BIT: {
                    auto const *p = asUint16();
                    for (size_t i = 0; i < mCount; ++i) {
                        ss << p[i];
                        ss << ", ";
                    }
                    break;
                }
                case SDL_GPU_INDEXELEMENTSIZE_32BIT: {
                    auto const *p = asUint32();
                    for (size_t i = 0; i < mCount; ++i) {
                        ss << p[i];
                        ss << ", ";
                    }
                    break;
                }
            }
            for (size_t i = 0; i < mCount; ++i) {
                ss << ", ";
            }
            ss << "]";
            return ss.str();
        }

    private:
        size_t const mCount;
        std::unique_ptr<uint8_t const> mBuffer;

    public:
        SDL_GPUIndexElementSize const mElementSize;
    };

    template<>
    inline uint16_t const *IndexList::as() const {
        return asUint16();
    }

    template<>
    inline uint32_t const *IndexList::as() const {
        return asUint32();
    }

    struct MeshData {
        [[nodiscard]] uint32_t getVertexBufferSize() const {
            return boost::safe_numerics::checked::cast<uint32_t>(mVertices.size() * sizeof(VertexData));
        }

        [[nodiscard]] uint32_t getIndexBufferSize() const {
            return boost::safe_numerics::checked::cast<uint32_t>(mIndices.bufferSize());
        }

        [[nodiscard]] std::string toString() const {
            std::stringstream ss;
            ss << "MeshData {\n";
            ss << "\tmVertices(";
            ss << mVertices.size();
            ss << ") = [\n";
            for (const auto &v: mVertices) {
                ss << "\t\t";
                ss << v.toString();
                ss << ",\n";
            }
            ss << "\t],\n";
            ss << "\tmIndices(";
            ss << mIndices.count();
            ss << ") = ";
            ss << mIndices.toString();
            ss << "\n";
            ss << "}";
            return ss.str();
        }

        std::vector<VertexData> const mVertices;
        IndexList const mIndices;
    };

    struct MeshDataBuilder {

        MeshData build() {
            if (mIndices16.empty() && mIndices32.empty()) {
                throw std::runtime_error("MeshDataBuilder: no indices set");
            }
            if (mIndices32.size() > 0) {
                return {
                        .mVertices = std::move(mVertices),
                        .mIndices = IndexList(mIndices32),
                };
            }
            return {
                    .mVertices = std::move(mVertices),
                    .mIndices = IndexList(mIndices16),
            };
        }

        void addIndex(int32_t index) {
            if (mIndices32.size() > 0) {
                mIndices32.push_back(index);
            } else if (index <= std::numeric_limits<uint16_t>::max()) {
                mIndices16.push_back(index);
            } else {
                std::ranges::move(mIndices16, std::back_inserter(mIndices32));
                mIndices16.clear();
            }
        }

        std::vector<VertexData> mVertices;

    private:
        std::vector<uint16_t> mIndices16;
        std::vector<uint32_t> mIndices32;
    };
} // namespace gfx_testing::shader
