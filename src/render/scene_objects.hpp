#pragma once

#include <ecs/ecs.hpp>
#include <game.hpp>
#include <render/debug_normals.hpp>
#include <render/render_object.hpp>

namespace gfx_testing::render {
    static constexpr glm::vec3 INITIAL_LIGHT_POSITION(2, 2, 2);
    static constexpr glm::vec3 PROP_OBJECTS_POSITION(0, 0, 0);
    static constexpr glm::vec3 LANDSCAPE_POSITION(0, 0, -15);
    static constexpr glm::vec3 LANDSCAPE_SCALE(20, 20, .5);
    static constexpr glm::vec3 TEXTURE_OBJECT_POSITION(-5, -5, 0);
    static constexpr glm::vec3 TEXTURE_OBJECT_SCALE(2);

    class SceneObjects {

    public:
        explicit SceneObjects(game::GameContext &gameContext);

        void update() const;

        bool hasDebugNormals() const { return mDebugNormals.has_value(); }

        void toggleDebugNormals(bool enable);

    private:
        game::GameContext &mGameContext;
        ecs::EntityRef<RenderObject> mPropObjects;
        util::ref_opt<DebugNormals> mDebugNormals;
    };
} // namespace gfx_testing::render
