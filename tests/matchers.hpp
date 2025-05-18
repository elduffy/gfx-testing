#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>
#include <glm/vec3.hpp>
#include <shader/shader_models.hpp>

// Keep last
#include <catch2/matchers/catch_matchers_templated.hpp>

namespace gfx_testing::test {
    template<int N>
    struct VecWithinAbsMatcher : Catch::Matchers::MatcherGenericBase {
        using vecN = glm::vec<N, float>;

        VecWithinAbsMatcher(vecN target, float margin) : mTarget(target), mMargin(margin) {}

        bool match(vecN const &other) const {
            bool matches = true;
            for (auto i = 0; i < N; ++i) {
                matches &= other[i] + mMargin >= mTarget[i] && mTarget[i] + mMargin >= other[i];
            }
            return matches;
        }

        std::string describe() const override {
            std::stringstream ss;
            ss << mTarget;
            return std::format("Within {} of {}", mMargin, ss.str());
        }

        vecN mTarget;
        float mMargin;
    };

    struct VertexDataWithinAbsMatcher : Catch::Matchers::MatcherGenericBase {
        VertexDataWithinAbsMatcher(shader::VertexData target, float margin) : mTarget(target), mMargin(margin) {}

        bool match(shader::VertexData const &other) const {
            return VecWithinAbsMatcher<3>(mTarget.mPosition, mMargin).match(other.mPosition) &&
                   VecWithinAbsMatcher<2>(mTarget.mUv, mMargin).match(other.mUv) &&
                   VecWithinAbsMatcher<3>(mTarget.mNormal, mMargin).match(other.mNormal) &&
                   VecWithinAbsMatcher<4>(mTarget.mColor, mMargin).match(other.mColor);
        }

        std::string describe() const override { return std::format("Within {} of {}", mMargin, mTarget.toString()); }

        shader::VertexData mTarget;
        float mMargin;
    };

    template<int N>
    auto WithinAbs(glm::vec<N, float> target, float margin) {
        return VecWithinAbsMatcher(std::move(target), margin);
    }

    auto WithinAbs(shader::VertexData const &target, float margin) {
        return VertexDataWithinAbsMatcher(target, margin);
    }
} // namespace gfx_testing::test
