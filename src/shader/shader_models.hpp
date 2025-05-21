#pragma once
#include <SDL3/SDL.h>
#include <absl/log/check.h>
#include <array>
#include <boost/safe_numerics/checked_default.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gooch.frag.hpp>
#include <ostream>
#include <shader/util.hpp>
#include <shader/vertex.hpp>
#include <sstream>
#include <vector>


namespace gfx_testing::shader {

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
            CHECK_LE(lightPosWs.size(), MAX_NUM_LIGHTS) << "Too many lights";
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
} // namespace gfx_testing::shader
