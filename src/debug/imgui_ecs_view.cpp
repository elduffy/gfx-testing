#include <debug/imgui_ecs_view.hpp>
#include <debug/imgui_utils.hpp>
#include <imgui.h>
#include <render/render_object.hpp>

namespace gfx_testing::imgui {
    void ImGuiEcsView::render(render::Scene &scene) {
        auto &ecs = scene.getGameContext().getEcs();
        if (ImGui::TreeNodeEx("Root", ImGuiTreeNodeFlags_DefaultOpen)) {
            static constexpr ImGuiTableFlags tableFlags =
                    ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable |
                    ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchProp;
            if (ImGui::BeginTable("ecs", 4, tableFlags)) {
                static constexpr ImGuiTableColumnFlags defaultColumnFlags =
                        ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch;
                ImGui::TableSetupColumn("ID", defaultColumnFlags);
                ImGui::TableSetupColumn("Name", defaultColumnFlags);
                ImGui::TableSetupColumn("Parent ID", defaultColumnFlags);
                ImGui::TableSetupColumn("Components", defaultColumnFlags);
                ImGui::TableHeadersRow();

                ecs.mRegistry.sort<entt::entity>([](entt::entity const &lhs, entt::entity const &rhs) {
                    return ecs::EntityId::getId(lhs) < ecs::EntityId::getId(rhs);
                });
                for (auto const entity: ecs.mRegistry.view<entt::entity>()) {
                    auto const entityId = ecs.get(entity);
                    ImGui::PushID(entityId.getId());
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%u", entityId.getId());
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", entityId.getName());
                    ImGui::TableNextColumn();
                    if (auto const parentOpt = entityId.getParent(); parentOpt.has_value()) {
                        ImGui::Text("%u", parentOpt->getId());
                    } else {
                        ImGui::TextDisabled("--");
                    }

                    ImGui::TableNextColumn();
                    if (ImGui::TreeNode("RenderObject")) {
                        if (auto const *renderObject = ecs.mRegistry.try_get<render::RenderObject>(entity);
                            renderObject == nullptr) {
                            ImGui::TextDisabled("--");
                        } else {
                            ImGui::Value("Pipeline", pipeline::gfx::getName(renderObject->getPipelineName()));
                            ImGui::Value("Position", renderObject->getPositionWs());
                        }
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
    }
} // namespace gfx_testing::imgui
