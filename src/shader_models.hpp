#pragma once
#include <vector>
#include <boost/safe_numerics/checked_default.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#define SHADER_ALIGN alignas(16)

namespace gfx_testing::shader {
    struct VertexData {
        glm::vec3 mPosition;
        glm::vec3 mNormal;
        glm::vec4 mColor;

        [[nodiscard]] std::string toString() const {
            std::stringstream ss;
            ss << "pos = {" << mPosition.x << ", " << mPosition.y << ", " << mPosition.z << "}, ";
            ss << "norm = {" << mNormal.x << ", " << mNormal.y << ", " << mNormal.z << "}, ";
            ss << "col = {" << mColor.x << ", " << mColor.y << ", " << mColor.z << "}";
            return ss.str();
        }
    };

    struct MvpTransform {
        glm::mat4 mModel;
        glm::mat4 mView;
        glm::mat4 mProjection;
    };

    struct CameraLight {
        SHADER_ALIGN glm::vec3 mCameraPosWs;
        SHADER_ALIGN glm::vec3 mLightPosWs;
    };

    struct MeshData {
        std::vector<VertexData> mVertices;
        std::vector<uint16_t> mIndices;

        [[nodiscard]] uint32_t getVertexBufferSize() const {
            return boost::safe_numerics::checked::cast<uint32_t>(mVertices.size() * sizeof(VertexData));
        }

        [[nodiscard]] uint32_t getIndexBufferSize() const {
            return boost::safe_numerics::checked::cast<uint32_t>(mIndices.size() * sizeof(uint16_t));
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
            ss << mIndices.size();
            ss << ") = [";
            for (const auto &i: mIndices) {
                ss << i;
                ss << ", ";
            }
            ss << "]\n";
            ss << "}";
            return ss.str();
        }
    };

    struct GoochParams {
        SHADER_ALIGN glm::vec3 mCoolColor;
        SHADER_ALIGN glm::vec3 mWarmColor;
    };
}
