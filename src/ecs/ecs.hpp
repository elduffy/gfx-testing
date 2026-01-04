#pragma once

#include <entt/entt.hpp>
#include <render/render_object.hpp>
#include <util/ref.hpp>

namespace gfx_testing::ecs {
    class Ecs {
    public:
        std::vector<util::cref<render::RenderObject>> getRenderObjects(pipeline::gfx::PipelineName pipelineName) const;
        void addRenderObject(render::RenderObject const &renderObject);

        entt::registry mRegistry;
    };
} // namespace gfx_testing::ecs
