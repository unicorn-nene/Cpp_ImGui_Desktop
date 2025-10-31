#include <exception>
#include <iostream>
#include <string_view>
#include <tuple>

#include <fmt/core.h>
#include <fmt/format.h>
#ifdef format
#undef format
#endif
#include <imgui.h>
#include <implot.h>

#include "Desktop.hpp"
#include "UiHelpers.hpp"

void Desktop::Draw(std::string_view label, bool *)
{
    ImGui::SetNextWindowSize(s_mainWindowSize);
    ImGui::SetNextWindowPos(s_mainWindowPos);

    ImGui::Begin(label.data(),
                 nullptr,
                 (s_mainWindowFlags | ImGuiWindowFlags_NoInputs |
                  ImGuiWindowFlags_NoTitleBar));

    DrawBackground();
    DrawDesktop();
    DrawTaskbar();

    ImGui::End();
}

/**
 * @brief 绘制桌面背景
 *
 * 功能说明：
 * - 设置光标位置到窗口左上角；
 * - 使用指定路径加载背景图片（bg.png）；
 * - 使用 ImGui::Image() 绘制整张背景；
 * - 再次重置光标位置，避免后续控件布局错位。
 *
 * @note
 * - PROJECT_PATH 通常在 CMakeLists.txt 中定义；
 * - 该背景不会随窗口大小自适应（如需可添加 ImGui::GetWindowSize() 动态调整）
 *
 */
void Desktop::DrawBackground()
{
    ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
    // const auto image_filepath = fmt::format("{}{}", PROJECT_PATH, "/images/bg.png");
    // LoadAndDisplayImage(image_filepath);
    ImGui::SetCursorPos(ImVec2(0.0f, 2.0f));
}


/**
 * @brief 绘制桌面图标区域
 *
 * 遍历所有 Icon 对象，并调用其 Draw() 方法。
 * 每个 Icon 会单独绘制一个小窗口（包含图标按钮与应用启动逻辑）。
 *
 * 作用类似于“桌面快捷方式”的渲染。
 */
void Desktop::DrawDesktop()
{
    for (auto &icon : icons)
        icon.Draw();
}


/**
 * @brief  绘制桌面任务栏（底部固定区域）
 *
 * - 固定在窗口底部 (y = 680)，宽度为 1280.
 *
 * - 包含三个主要按钮：
 *   1. “All Icons”：显示所有程序图标的弹出窗口；
 *   2. “Theme”：打开主题颜色设置面板；
 *   3. 当前时间按钮：点击后显示时钟窗口。
 * @note 使用 ImGui 的弹出窗口机制 (PopupModal) 管理子菜单。
 *
 */
void Desktop::DrawTaskbar()
{
    constexpr static auto button_flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    static auto open_taskbar = false;

    // 固定任务栏位置和尺寸
    ImGui::SetNextWindowPos(ImVec2(0.0f, 680.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(1280.0f, 40.0f), ImGuiCond_Always);

    ImGui::Begin("Taskbar", nullptr, button_flags);

    // 左侧: 程序启动菜单按钮
    ImGui::SetCursorPosX(0.0f);
    if (ImGui::Button("All Icons", ImVec2(100.0f, 30.0f)))
    {
        ImGui::OpenPopup("My Programes");
        open_taskbar = true;
    }

    // 若菜单已经打开,则绘制图标列表窗口
    if (open_taskbar)
        ShowIconList(&open_taskbar);

    ImGui::SameLine();

    // 中间: 主题设置按钮
    static auto theme_open = false;
    if (ImGui::Button("Theme", ImVec2(100.0f, 30.0f)) || theme_open)
    {
        theme_open = true;
        DrawColorsSettings(&theme_open);
    }

    ImGui::SameLine();

    //右侧: 时钟显示
    ImGui::SetCursorPosX(s_mainWindowSize.x - 100.0f);

    static auto clock_open = false;
    clock_.GetTime();
    const auto time = fmt::format("{}:{}", clock_.hrs, clock_.mins);
    if (ImGui::Button(time.data(), ImVec2(100.0f, 30.0f)) || clock_open)
    {
        clock_.Draw("clockWindow");
        clock_open = true;
    }

    // 点击空白处关闭时钟窗口
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        clock_open = false;

    ImGui::End();
}

/**
 * @brief 绘制"所有程序"弹出列表
 *
 * @param open 指向控制弹窗状态的布尔指针
 */
void Desktop::ShowIconList(bool *open)
{
    const auto icon_count = static_cast<float>(icons.size());
    const auto selectable_height = ImGui::GetTextLineHeightWithSpacing();
    const auto popup_height = selectable_height * icon_count + 40.0f;

    //弹窗列表位于任务栏上方
    ImGui::SetNextWindowPos(ImVec2(0.0f, 680.0f - popup_height),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(100.0f, popup_height), ImGuiCond_Always);

    if (ImGui::BeginPopupModal("My Programs", open, ImGuiWindowFlags_NoResize))
    {
        for (auto &icon : icons)
        {
            if (ImGui::Selectable(icon.label_.data()))
            {
                icon.popupOpen_ = true;
                icon.base_->Draw(icon.label_, &icon.popupOpen_);
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }
}


/**
 * @brief 绘制单个桌面图标
 * - 每个 Icon 是一个独立的 ImGui 窗口
 * - 使用 '###' 前缀隐藏标题(只保留唯一ID)
 * - 点击按钮后打开对应应用窗口
 * - 若 popupOpen 为ture,则保持窗口出于打开状态
 *
 * @warning
 * - ImGui 的每个窗口需要唯一ID,否则可能冲突
 *
 */
void Desktop::Icon::Draw()
{
    constexpr static auto flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    // 初次显示时设置默认设置
    ImGui::SetNextWindowPos(position_, ImGuiCond_Always);
    ImGui::Begin(fmt::format("###{}", label_).data(), nullptr, flags);

    // 图标按钮,点击后打开应用窗口
    if (ImGui::Button(label_.data(), ImVec2(100.0f, 50.0f)) || popupOpen_)
    {
        popupOpen_ = true;
        base_->Draw(label_, &popupOpen_);
    }

    ImGui::End();
}


/**
 * @brief 渲染入口函数。
 * @param window_class Desktop 对象引用。
 *
 * 在主渲染循环中被调用，用于绘制整个桌面系统界面。
 */
void render(Desktop &window_class)
{
    window_class.Draw("Desktop");
}
