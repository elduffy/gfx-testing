#include <ecs/ecs.hpp>
#include <render/render_object.hpp>
#include <util/ref.hpp>

namespace gfx_testing::ecs {
    EntityId Ecs::create(char const *name) {
        EntityId id{
                .mEcs = *this,
                .mEntity = mRegistry.create(),
        };
        SDL_Log("Created entity %u", id.getId());
        id.emplace<EntityBase>(name);
        return id;
    }

    EntityId Ecs::get(entt::entity entity) { return {*this, entity}; }

    void Ecs::destroy(EntityId id) {
        SDL_Log("Destroying entity %u", id.getId());
        mRegistry.destroy(id.mEntity);
    }

    std::optional<EntityId> EntityId::getParent() const {
        auto const &base = getEntityBase();
        return util::transform(base.getParent(), [this](entt::entity parent) { return EntityId{mEcs, parent}; });
    }

    EntityBase &getEntityBaseInternal(EntityId entityId) {
        return entityId.mEcs.mRegistry.get<EntityBase>(entityId.mEntity);
    }

    void EntityId::setParent(EntityId parent) const { getEntityBaseInternal(*this).setParent(parent.mEntity); }

    EntityBase const &EntityId::getEntityBase() const { return getEntityBaseInternal(*this); }
} // namespace gfx_testing::ecs
