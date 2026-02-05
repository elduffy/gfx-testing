#pragma once

#include <imgui.h>
#include <render/scene.hpp>

namespace ImGui {
    IMGUI_API void Value(const char *prefix, glm::vec3 const &v);
    IMGUI_API void Value(const char *prefix, char const *v);
} // namespace ImGui


namespace gfx_testing::imgui {
    float *getVectorData(glm::vec3 &v);
} // namespace gfx_testing::imgui
