#pragma once

#include <entt/entt.hpp>
#include <render/render_object.hpp>
#include <util/ref.hpp>

namespace gfx_testing::ecs {
    struct EntityId;

    template<typename T>
    struct EntityRef;

    class Ecs {
    public:
        EntityId create();

        template<typename T, typename... Args>
        EntityRef<T> createAndEmplace(Args &&...args);

        std::vector<util::cref<render::RenderObject>> getRenderObjects(pipeline::gfx::PipelineName pipelineName) const;

        void addRenderObject(render::RenderObject const &renderObject);

        entt::registry mRegistry;
    };

    struct EntityId {
        Ecs &mEcs;
        entt::entity mEntity;

        uint32_t getId() const {
            static_assert(std::is_same_v<ENTT_ID_TYPE, uint32_t>);
            return static_cast<uint32_t>(mEntity);
        };

        template<typename T, typename... Args>
        T &emplace(Args &&...args) {
            return mEcs.mRegistry.emplace<T>(mEntity, std::forward<Args>(args)...);
        }

        template<typename T>
        T &get() const {
            return mEcs.mRegistry.get<T>(mEntity);
        }

        template<typename T>
        util::ref_opt<T> tryGet() const {
            auto *ptr = mEcs.mRegistry.try_get<T>(mEntity);
            return util::ref_opt<T>(ptr);
        }
    };
    static_assert(sizeof(EntityId) <= 32);

    template<typename T>
    struct EntityRef {
        EntityId mId;
        T &mRef;
    };

    template<typename T, typename... Args>
    EntityRef<T> Ecs::createAndEmplace(Args &&...args) {
        auto entityId = create();
        auto &obj = entityId.emplace<T>(std::forward<Args>(args)...);
        return {entityId, obj};
    }
} // namespace gfx_testing::ecs
