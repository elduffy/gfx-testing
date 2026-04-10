#pragma once
#include <optional>

#include <entt/entity/entity.hpp>

namespace gfx_testing::ecs {
    class EntityBase {
    public:
        explicit EntityBase(const char *name) : mName(name) {}

        // Returns the former parent entity
        std::optional<entt::entity> setParent(entt::entity entity) {
            auto const old = mParent;
            mParent = entity;
            return old;
        }

        std::optional<entt::entity> getParent() const { return mParent; }

        char const *getName() const { return mName; }

    private:
        char const *mName;
        std::optional<entt::entity> mParent;
    };
} // namespace gfx_testing::ecs
