#pragma once

#include <ecs/ecs.hpp>
#include <game.hpp>
#include <render/debug_axes.hpp>
#include <render/debug_normals.hpp>
#include <render/point_light.hpp>
#include <render/render_object.hpp>
#include <render/sky_box.hpp>
#include <util/ref.hpp>

namespace gfx_testing::render {
    static constexpr glm::vec3 INITIAL_LIGHT_POSITION(2, 2, 2);
    static constexpr glm::vec3 PROP_OBJECTS_POSITION(0, 0, 0);
    static constexpr glm::vec3 LANDSCAPE_POSITION(0, 0, -15);
    static constexpr glm::vec3 LANDSCAPE_SCALE(20, 20, .5);
    static constexpr glm::vec3 TEXTURE_OBJECT_POSITION(-5, -5, 0);
    static constexpr glm::vec3 TEXTURE_OBJECT_SCALE(2);

    class SceneObjects {

    public:
        explicit SceneObjects(game::GameContext &gameContext, ecs::Ecs &ecs);

        void update();

        RenderObject const &getPropObjects() { return mPropObjects; }

        DebugNormals const &getDebugNormals() { return mDebugNormals; }

        void toggleDebugNormals(bool enable);

    private:
        std::vector<util::cref_vec<RenderObject>> calculateRenderObjectsByPipeline() const;

        game::GameContext &mGameContext;
        SkyBox mSkyBox;
        RenderObject mPropObjects;
        RenderObject mLandscape;
        RenderObject mTextureObject;
        DebugAxes mDebugAxes;
        DebugNormals mDebugNormals;

    public:
        std::vector<PointLight> mPointLights;
    };
} // namespace gfx_testing::render
