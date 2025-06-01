#pragma once
#include <array>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <ostream>
#include <shader/util.hpp>
#include <sstream>


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
            ss << "col = {" << mColor.x << ", " << mColor.y << ", " << mColor.z << ", " << mColor.w << "}";
            return ss.str();
        }

        friend bool operator==(const VertexData &lhs, const VertexData &rhs) {
            return lhs.mPosition == rhs.mPosition && lhs.mUv == rhs.mUv && lhs.mNormal == rhs.mNormal &&
                   lhs.mColor == rhs.mColor;
        }

        friend std::ostream &operator<<(std::ostream &os, const VertexData &obj) { return os << obj.toString(); }

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
} // namespace gfx_testing::shader
