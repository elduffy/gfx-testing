#pragma once

#include <entt/entt.hpp>
#include <render/render_object.hpp>
#include <util/ref.hpp>

namespace gfx_testing::ecs {
    struct EntityId;

    class Ecs {
    public:
        EntityId create();

        std::vector<util::cref<render::RenderObject>> getRenderObjects(pipeline::gfx::PipelineName pipelineName) const;

        void addRenderObject(render::RenderObject const &renderObject);

        entt::registry mRegistry;
    };

    struct EntityId {
        Ecs &mEcs;
        entt::entity mEntity;

        template<typename T, typename... Args>
        T &emplace(Args &&...args) {
            return mEcs.mRegistry.emplace<T>(mEntity, std::forward<Args>(args)...);
        }
    };
    static_assert(sizeof(EntityId) <= 32);
} // namespace gfx_testing::ecs
