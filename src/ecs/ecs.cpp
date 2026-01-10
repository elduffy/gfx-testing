#include <ecs/ecs.hpp>
#include <render/render_object.hpp>
#include <util/ref.hpp>

namespace gfx_testing::ecs {
    EntityId Ecs::create() {
        const EntityId id{
                .mEcs = *this,
                .mEntity = mRegistry.create(),
        };
        SDL_Log("Created entity %u", id.getId());
        return id;
    }

    void Ecs::destroy(EntityId id) {
        SDL_Log("Destroying entity %u", id.getId());
        mRegistry.destroy(id.mEntity);
    }

    util::ref_opt<EntityId> EntityId::getParent() const {
        auto *parent = mEcs.mRegistry.try_get<ParentEntity>(mEntity);
        if (parent == nullptr) {
            return {};
        }
        return {parent->mParent};
    }

    void EntityId::setParent(EntityId parent) const { mEcs.mRegistry.emplace<ParentEntity>(mEntity, parent); }
} // namespace gfx_testing::ecs
