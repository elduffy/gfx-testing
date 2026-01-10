#pragma once

#include <pipeline/gfx/pipeline_definition.hpp>
#include <render/render_object.hpp>

namespace gfx_testing::ecs::render {
    template<pipeline::gfx::PipelineName>
    struct PipelineNameTag {};

    template<pipeline::gfx::PipelineName pipelineName, typename... Args>
    gfx_testing::render::RenderObject &emplaceRenderObject(EntityId entityId, Args &&...args) {
        gfx_testing::render::RenderObject &renderObj =
                entityId.emplace<gfx_testing::render::RenderObject>(std::forward<Args>(args)...);
        CHECK_EQ(pipelineName, renderObj.getPipelineName());
        entityId.emplace<PipelineNameTag<pipelineName>>();
        return renderObj;
    }

    template<pipeline::gfx::PipelineName pipelineName, typename... Args>
    EntityRef<gfx_testing::render::RenderObject> &createAndEmplaceRenderObject(Ecs &ecs, Args &&...args) {
        auto entityRef = ecs.createAndEmplace<gfx_testing::render::RenderObject>(std::forward<Args>(args)...);
        CHECK_EQ(pipelineName, entityRef.mRef.getPipelineName());
        entityRef.mId.template emplace<PipelineNameTag<pipelineName>>();
        return entityRef;
    }

    template<pipeline::gfx::PipelineName pipelineName, typename Fn>
    void eachRenderObject(Ecs const &ecs, Fn const &fn) {
        auto const view = ecs.mRegistry.view<gfx_testing::render::RenderObject, PipelineNameTag<pipelineName>>();
        view.each([&fn](gfx_testing::render::RenderObject const &renderObject) { fn(renderObject); });
    }
} // namespace gfx_testing::ecs::render
