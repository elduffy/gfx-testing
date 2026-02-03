#pragma once

#include <ecs/entity.hpp>
#include <entt/entt.hpp>
#include <util/ref.hpp>
#include <util/util.hpp>

namespace gfx_testing::ecs {
    struct EntityId;

    class Ecs {
    public:
        EntityId create(char const *name = nullptr);
        EntityId get(entt::entity entity);

        void destroy(EntityId id);

        entt::registry mRegistry;
    };

    struct EntityId {
        Ecs &mEcs;
        entt::entity mEntity;

        static uint32_t getId(entt::entity entity) {
            static_assert(std::is_same_v<ENTT_ID_TYPE, uint32_t>);
            return static_cast<uint32_t>(entity);
        }

        void destroy() const { mEcs.destroy(*this); }

        uint32_t getId() const { return getId(mEntity); };

        std::optional<EntityId> getParent() const;

        void setParent(EntityId parent) const;

        EntityBase const &getEntityBase() const;

        template<typename T, typename... Args>
        decltype(auto) emplace(Args &&...args) {
            return mEcs.mRegistry.emplace<T>(mEntity, std::forward<Args>(args)...);
        }

        template<typename T>
        T &get() const {
            return mEcs.mRegistry.get<T>(mEntity);
        }

        char const *getName() const {
            auto const &baseEntity = mEcs.mRegistry.get<EntityBase>(mEntity);
            return baseEntity.getName();
        }

        template<typename T>
        util::ref_opt<T> tryGet() const {
            auto *ptr = mEcs.mRegistry.try_get<T>(mEntity);
            return util::ref_opt<T>(ptr);
        }
    };
    static_assert(sizeof(EntityId) <= 32);
} // namespace gfx_testing::ecs
