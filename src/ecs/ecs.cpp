#include <ecs/ecs.hpp>
#include <render/render_object.hpp>
#include <util/ref.hpp>

namespace gfx_testing::ecs {
    EntityId Ecs::create() {
        return {
                .mEcs = *this,
                .mEntity = mRegistry.create(),
        };
    }
    void Ecs::destroy(EntityId id) { mRegistry.destroy(id.mEntity); }

    util::ref_opt<EntityId> EntityId::getParent() const {
        auto *parent = mEcs.mRegistry.try_get<ParentEntity>(mEntity);
        if (parent == nullptr) {
            return {};
        }
        return {parent->mParent};
    }

    void EntityId::setParent(EntityId parent) const { mEcs.mRegistry.emplace<ParentEntity>(mEntity, parent); }

    std::vector<util::cref<render::RenderObject>>
    Ecs::getRenderObjects(pipeline::gfx::PipelineName pipelineName) const {
        std::vector<util::cref<render::RenderObject>> result;
        {
            // Non-ECS managed render objects. TODO: remove this
            auto const view = mRegistry.view<util::cref<render::RenderObject>>();
            for (const auto entity: view) {
                if (auto const &renderObject = view.get<util::cref<render::RenderObject>>(entity);
                    renderObject.get().getPipelineName() == pipelineName) {
                    result.emplace_back(renderObject);
                }
            }
        }
        {
            // ECS-managed render objects
            auto const view = mRegistry.view<render::RenderObject>();
            for (const auto entity: view) {
                if (auto const &renderObject = view.get<render::RenderObject>(entity);
                    renderObject.getPipelineName() == pipelineName) {
                    result.emplace_back(renderObject);
                }
            }
        }
        return result;
    }

    void Ecs::addRenderObject(render::RenderObject const &renderObject) {
        const auto entity = mRegistry.create();
        mRegistry.emplace<util::cref<render::RenderObject>>(entity, renderObject);
    }
} // namespace gfx_testing::ecs
