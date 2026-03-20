#pragma once

#include <filesystem>
#include <glm/vec3.hpp>
#include <string>
#include <variant>
#include <vector>

namespace gfx_testing::io {

    struct MeshObjectDef {
        std::string mName;
        std::string mModel;
        std::string mPipeline;
        glm::vec3 mPosition{0, 0, 0};
        glm::vec3 mScale{1, 1, 1};
        bool mUntextured{false};
        std::string mBehavior;
        float mBehaviorRadsPerSecond{0};
    };

    struct SkyboxDef {
        std::string mName;
        std::string mCubemap;
    };

    struct PointLightsDef {
        std::string mName;
        int mCount{3};
        std::string mModel;
        float mOrbitRadius{2.8284f};
    };

    using SceneObjectDef = std::variant<MeshObjectDef, SkyboxDef, PointLightsDef>;

    struct SceneDefinition {
        std::vector<SceneObjectDef> mObjects;
    };

    SceneDefinition loadSceneDefinition(std::filesystem::path const &path);

} // namespace gfx_testing::io
