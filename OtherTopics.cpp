#include <exception>
#include <iostream>
#include <string_view>
#include <tuple>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "OtherTopics.hpp"
#include "UiHelpers.hpp"

/**
 * @brief 绘制示例 UI 窗口
 *
 *   本函数展示了 ImGui 多种控件的使用示例，包括：
 * - TreeNode 树状折叠节点
 * - CollapsingHeader 折叠式面板
 * - TabBar 标签页切换
 *
 * 整体结构为：
 * ┌──────────────────────────────┐
 * │  TreeNode 区域                │
 * │  CollapsingHeader 区域        │
 * │  TabBar (包含两个标签页)        │
 * └──────────────────────────────┘
 * @param label
 * @param open
 */
void OtherTopics::Draw(std::string_view label, bool *open)
{
    // 设置窗口的固定位置和大小
    ImGui::SetNextWindowPos(s_mainWindowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(s_mainWindowSize, ImGuiCond_Always);

    // 创建主窗口(带标题栏)
    ImGui::Begin(label.data(), open, s_mainWindowFlags);

    // ================================
    //1.TreeNode
    if (ImGui::TreeNode("Tabbing"))
    {
        //static 缓冲区存储输入文本
        static char buf[32] = "Hello";

        // 三个独立输入框,使用相同缓冲区
        ImGui::InputText("1", buf, IM_ARRAYSIZE(buf));
        ImGui::InputText("2", buf, IM_ARRAYSIZE(buf));
        ImGui::InputText("3", buf, IM_ARRAYSIZE(buf));

        // TreeNode 标记
        ImGui::TreePop();
    }

    // ==============================
    // 2. 折叠式 Help 面板
    if (ImGui::CollapsingHeader("Help"))
    {
        // 简单文本显示,用于说明或提示
        ImGui::Text("1");
        ImGui::Text("2");
        ImGui::Text("3");
    }

    // ====================================
    // 3.标签页示例
    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
    {
        // 标签页1
        if (ImGui::BeginTabItem("Sizes1"))
        {
            ImGui::SeparatorText("Main11");
            ImGui::Text("1");

            ImGui::SeparatorText("Main21");
            ImGui::Text("2");
            ImGui::EndTabItem();
        }

        // 标签页2
        if (ImGui::BeginTabItem("Sizes2"))
        {
            ImGui::SeparatorText("Main12");
            ImGui::Text("1");

            ImGui::SeparatorText("Main22");
            ImGui::Text("2");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}
