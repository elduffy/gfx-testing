#pragma once

#include <imgui.h>
#include <render/scene.hpp>

namespace ImGui {
    IMGUI_API inline void Value(const char *prefix, glm::vec3 const &v) { Text("%s: %f %f %f", prefix, v.x, v.y, v.z); }
    IMGUI_API inline void Value(const char *prefix, char const *v) { Text("%s: %s", prefix, v); }
} // namespace ImGui
