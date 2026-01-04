#pragma once

#include <entt/entt.hpp>
#include <render/render_object.hpp>
#include <util/ref.hpp>

namespace gfx_testing::ecs {
    struct EntityId;

    template<typename T>
    struct EntityRef;

    struct ParentEntity;

    class Ecs {
    public:
        EntityId create();

        void destroy(EntityId id);

        template<typename T, typename... Args>
        EntityRef<T> createAndEmplace(Args &&...args);

        std::vector<util::cref<render::RenderObject>> getRenderObjects(pipeline::gfx::PipelineName pipelineName) const;

        void addRenderObject(render::RenderObject const &renderObject);

        // template<typename T>
        // EntityRef<T> findSingleton() const;

        entt::registry mRegistry;
    };

    struct EntityId {
        Ecs &mEcs;
        entt::entity mEntity;

        void destroy() const { mEcs.destroy(*this); }

        uint32_t getId() const {
            static_assert(std::is_same_v<ENTT_ID_TYPE, uint32_t>);
            return static_cast<uint32_t>(mEntity);
        };

        template<typename T>
        EntityRef<T> asEntityRef() const;

        util::ref_opt<EntityId> getParent() const;

        void setParent(EntityId parent) const;

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

    struct ParentEntity {
        EntityId mParent;
    };


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

    template<typename T>
    EntityRef<T> EntityId::asEntityRef() const {
        return {*this, get<T>()};
    }
} // namespace gfx_testing::ecs
