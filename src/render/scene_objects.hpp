#pragma once

#include <ecs/ecs.hpp>
#include <game.hpp>
#include <io/scene_loader.hpp>
#include <render/debug_normals.hpp>

namespace gfx_testing::render {

    struct RotateBehavior {
        glm::vec3 mAxis;
        float mRadsPerSecond;
    };

    class SceneObjects {

    public:
        SceneObjects(game::GameContext &gameContext, std::string const &sceneFilename);

        void update() const;

        bool hasDebugNormals() const { return mDebugNormals.has_value(); }

        void toggleDebugNormals(bool enable);

    private:
        game::GameContext &mGameContext;
        io::SceneDefinition mSceneDefinition;
        ecs::EntityId mPropObjects;
        util::ref_opt<DebugNormals> mDebugNormals;
    };
} // namespace gfx_testing::render
