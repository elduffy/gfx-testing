#pragma once

#include <entt/entt.hpp>
#include <util/ref.hpp>
#include <util/util.hpp>

namespace gfx_testing::ecs {
    struct EntityId;

    struct ParentEntity;

    class Ecs {
    public:
        EntityId create();

        void destroy(EntityId id);

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

        util::ref_opt<EntityId> getParent() const;

        void setParent(EntityId parent) const;

        template<typename T, typename... Args>
        decltype(auto) emplace(Args &&...args) {
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
} // namespace gfx_testing::ecs
