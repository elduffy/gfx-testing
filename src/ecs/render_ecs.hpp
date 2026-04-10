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
    EntityId createAndEmplaceRenderObject(Ecs &ecs, char const *name, Args &&...args) {
        auto entityId = ecs.create(name);
        emplaceRenderObject<pipelineName>(entityId, std::forward<Args>(args)...);
        return entityId;
    }

    template<typename... Args>
    EntityId createAndEmplaceRenderObjectDynamic(pipeline::gfx::PipelineName pipelineName, Ecs &ecs, char const *name,
                                                 Args &&...args) {
        switch (pipelineName) {
            case pipeline::gfx::PipelineName::SimpleColor:
                return createAndEmplaceRenderObject<pipeline::gfx::PipelineName::SimpleColor>(
                        ecs, name, std::forward<Args>(args)...);
            case pipeline::gfx::PipelineName::Gooch:
                return createAndEmplaceRenderObject<pipeline::gfx::PipelineName::Gooch>(ecs, name,
                                                                                        std::forward<Args>(args)...);
            case pipeline::gfx::PipelineName::Textured:
                return createAndEmplaceRenderObject<pipeline::gfx::PipelineName::Textured>(ecs, name,
                                                                                           std::forward<Args>(args)...);
            case pipeline::gfx::PipelineName::Lines:
                return createAndEmplaceRenderObject<pipeline::gfx::PipelineName::Lines>(ecs, name,
                                                                                        std::forward<Args>(args)...);
            case pipeline::gfx::PipelineName::Lambert:
                return createAndEmplaceRenderObject<pipeline::gfx::PipelineName::Lambert>(ecs, name,
                                                                                          std::forward<Args>(args)...);
            case pipeline::gfx::PipelineName::Skybox:
                return createAndEmplaceRenderObject<pipeline::gfx::PipelineName::Skybox>(ecs, name,
                                                                                         std::forward<Args>(args)...);
        }
        FAIL("Unknown pipeline name '{}'", static_cast<uint32_t>(pipelineName));
    }

    template<pipeline::gfx::PipelineName pipelineName, typename Fn>
    void eachRenderObject(Ecs const &ecs, Fn const &fn) {
        auto const view = ecs.mRegistry.view<gfx_testing::render::RenderObject, PipelineNameTag<pipelineName>>();
        view.each([&fn](gfx_testing::render::RenderObject const &renderObject) { fn(renderObject); });
    }
} // namespace gfx_testing::ecs::render
