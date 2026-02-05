#pragma once
#include <ecs/ecs.hpp>

namespace gfx_testing::imgui {
    class ImGuiEcsView {
    public:
        void render(ecs::Ecs &ecs);
    };
} // namespace gfx_testing::imgui
