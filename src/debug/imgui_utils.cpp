#pragma once

#include <debug/imgui_utils.hpp>
#include <imgui.h>
#include <render/scene.hpp>

IMGUI_API void ImGui::Value(const char *prefix, glm::vec3 const &v) { Text("%s: %f %f %f", prefix, v.x, v.y, v.z); }
IMGUI_API void ImGui::Value(const char *prefix, char const *v) { Text("%s: %s", prefix, v); }


float *gfx_testing::imgui::getVectorData(glm::vec3 &v) {
    static_assert(offsetof(glm::vec3, x) == 0);
    static_assert(sizeof(v.x) == sizeof(float));
    static_assert(offsetof(glm::vec3, y) == sizeof(float));
    static_assert(sizeof(v.y) == sizeof(float));
    static_assert(offsetof(glm::vec3, z) == offsetof(glm::vec3, y) + sizeof(float));
    static_assert(sizeof(v.z) == sizeof(float));
    return &v.x;
}
