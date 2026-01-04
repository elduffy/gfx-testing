#pragma once

#include <ecs/ecs.hpp>
#include <game.hpp>
#include <render/render_object.hpp>

namespace gfx_testing::render {
    class DebugNormals {
    public:
        struct Options {
            SHADER_ALIGN glm::vec3 mLineColor{0, 1, .72};
            float mLineLength = 0.1;
        };
        static_assert(sizeof(Options) == 16);

        static DebugNormals &create(ecs::EntityId target, game::GameContext const &gameContext, Options const &options);

        DebugNormals(ecs::EntityId entityId, game::GameContext const &gameContext,
                     const ecs::EntityRef<RenderObject> &target, Options const &options);

        void update() const;

        ecs::EntityId mEntityId;

    private:
        // std::optional<ecs::EntityRef<RenderObject>> mTargetObject;
    };
} // namespace gfx_testing::render
