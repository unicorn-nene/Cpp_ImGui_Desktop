#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "Paint.hpp"

void Paint::Draw(std::string_view label, bool *open)
{
    ImGui::SetNextWindowPos(s_mainWindowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(s_mainWindowSize, ImGuiCond_Always);

    ImGui::Begin(label.data(), open, s_mainWindowFlags);

    DrawMenu();
    DrawCanvas();

    ImGui::End();
}

void Paint::DrawMenu()
{
    // 检测快捷键
    const auto ctrl_pressed = ImGui::GetIO().KeyCtrl;
    const auto s_pressed = ImGui::IsKeyPressed(ImGuiKey_S);
    const auto l_pressed = ImGui::IsKeyPressed(ImGuiKey_L);

    if (ImGui::Button("Save") || (ctrl_pressed && s_pressed))
    {
        ImGui::OpenPopup("Save Image");
    }

    ImGui::SameLine();

    if (ImGui::Button("Load") || (ctrl_pressed && l_pressed))
    {
        ImGui::OpenPopup("Load Image");
    }

    ImGui::SameLine();

    if (ImGui::Button("Clear"))
    {
        ClearCanvas();
    }

    DrawColorButtons();
    DrawSizeSettings();

    DrawSavePopup();
    DrawLoadPopup();
}

void Paint::DrawSavePopup()
{
    const auto esc_pressed = ImGui::IsKeyPressed(ImGuiKey_Escape);

    // 设置弹窗大小与位置
    ImGui::SetNextWindowSize(s_popUpSize);
    ImGui::SetNextWindowPos(s_popUpPos);

    // BeginPopupModal () 弹出模态窗口(阻止用户与当前程序其他界面交互)
    if (ImGui::BeginPopupModal("Save Image", nullptr, s_popUpFlags))
    {
        // 显示输入框 输入文件名
        ImGui::Text("Filename", filenameBuffer_, sizeof(filenameBuffer_));

        // 点击Save -> 保存文件并关闭弹窗
        if (ImGui::Button("Save", s_popUpButtonSize))
        {
            SaveToImageFile(filenameBuffer_);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        // 点击 Cancel 或 Esc->直接关闭弹窗
        if (ImGui::Button("Cancel", s_popUpButtonSize) || esc_pressed)
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Paint::DrawLoadPopup()
{
    const auto esc_pressed = ImGui::IsKeyPressed(ImGuiKey_Escape);

    ImGui::SetNextWindowSize(s_popUpSize);
    ImGui::SetNextWindowPos(s_popUpPos);

    if (ImGui::BeginPopupModal("Load Image", nullptr, s_popUpFlags))
    {
        ImGui::InputText("Load File", filenameBuffer_, sizeof(filenameBuffer_));

        if (ImGui::Button("Load", s_popUpButtonSize))
        {
            LoadFromImageFile(filenameBuffer_);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", s_popUpButtonSize))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}


void Paint::DrawCanvas()
{
    canvasPos_ = ImGui::GetCursorPos();
    // 设置画布边框的厚度
    const auto border_thickness = 1.5f;
    // 计算画布区域的整体大小(包含边框)
    const auto button_size = ImVec2(canvasSize_.x + 2 * border_thickness,
                                    canvasSize_.y + 2 * border_thickness);
    // 使用一个不可见的按钮作为画布区域
    ImGui::InvisibleButton("##canvas", button_size);

    // 当前鼠标位置
    const auto mouse_pos = ImGui::GetMousePos();
    // 当前鼠标是否悬停在画布上
    const auto is_mouse_horvering = ImGui::IsItemHovered();

    if (is_mouse_horvering && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        // 计算点在画布内的相对位置（减去画布偏移和边框）
        const auto point =
            ImVec2(mouse_pos.x - border_thickness - canvasPos_.x,
                   mouse_pos.y - border_thickness - canvasPos_.y);

        points_.push_back(
            std::make_tuple(point, currentDrawColor_, pointDrawSize_));
    }

    // 获取当前窗口的渲染绘制列表
    auto *draw_list = ImGui::GetWindowDrawList();

    for (const auto &[point, color, size] : points_)
    { // 计算点在窗口中的绝对坐标（加上画布偏移和边框厚度）
        const auto pos = ImVec2(point.x + border_thickness + canvasPos_.x,
                                point.y + border_thickness + canvasPos_.y);

        // 绘制一个填充圆点
        draw_list->AddCircleFilled(pos, size, color);
    }

    // 绘制画布边框
    const auto border_min = canvasPos_;
    const auto border_max =
        ImVec2(canvasPos_.x + button_size.x - border_thickness,
               canvasPos_.y + button_size.y - border_thickness);

    draw_list->AddRect(border_min,
                       border_max,
                       IM_COL32(255, 255, 255, 255),
                       0.0F,
                       ImDrawFlags_RoundCornersAll,
                       border_thickness);
}

void Paint::DrawColorButtons()
{
    // 检查当前颜色是否等于某个预设颜色
    const auto selected_red = currentDrawColor_ == ImColor(255, 0, 0);
    const auto selected_green = currentDrawColor_ == ImColor(0, 255, 0);
    const auto selected_blue = currentDrawColor_ == ImColor(0, 0, 255);
    const auto selected_white = currentDrawColor_ == ImColor(255, 255, 255);
    // 如果当前颜色不是任何预设色，则进入“自定义颜色”状态
    const auto none_preset_color =
        !selected_red && !selected_green && !selected_blue && !selected_white;
    constexpr static auto orange = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);

    if (selected_red)
        ImGui::PushStyleColor(ImGuiCol_Button, orange);
    if (ImGui::Button("Red"))
        currentDrawColor_ = ImColor(255, 0, 0);
    if (selected_red)
        ImGui::PopStyleColor();

    ImGui::SameLine();
    if (selected_green)
        ImGui::PushStyleColor(ImGuiCol_Button, orange);
    if (ImGui::Button("Green"))
        currentDrawColor_ = ImColor(0, 255, 0);
    if (selected_green)
        ImGui::PopStyleColor();

    ImGui::SameLine();
    if (selected_blue)
        ImGui::PushStyleColor(ImGuiCol_Button, orange);
    if (ImGui::Button("Blue"))
        currentDrawColor_ = ImColor(0, 0, 255);
    if (selected_blue)
        ImGui::PopStyleColor();

    ImGui::SameLine();
    if (selected_white)
        ImGui::PushStyleColor(ImGuiCol_Button, orange);
    if (ImGui::Button("White"))
        currentDrawColor_ = ImColor(255, 255, 255);
    if (selected_white)
        ImGui::PopStyleColor();

    ImGui::SameLine();
    if (none_preset_color)
        ImGui::PushStyleColor(ImGuiCol_Button, orange);
    if (ImGui::Button("Choose"))
        ImGui::OpenPopup("Color Picker");
    if (ImGui::BeginPopup("Color Picker"))
    {
        ImGui::ColorPicker3("##picker",
                            reinterpret_cast<float *>(&currentDrawColor_));
        ImGui::EndPopup();
    }

    if (none_preset_color)
        ImGui::PopStyleColor();
}

void Paint::DrawSizeSettings()
{
    ImGui::Text("Draw Size");
    ImGui::SameLine();

    ImGui::PushItemWidth(canvasSize_.x - ImGui::GetCursorPosX());
    ImGui::SliderFloat("##drawSize", &pointDrawSize_, 1.0f, 10.0f);
    ImGui::PopItemWidth();
}

void Paint::SaveToImageFile(std::string_view filename)
{
    auto out = std::ofstream(filename.data(), std::ios::binary);
    if (!out || !out.is_open())
        return;

    const auto point_count = points_.size();
    out.write(reinterpret_cast<const char *>(&point_count),
              sizeof(point_count));

    for (const auto &[point, color, size] : points_)
    {
        out.write(reinterpret_cast<const char *>(&point), sizeof(point));
        out.write(reinterpret_cast<const char *>(&color), sizeof(color));
        out.write(reinterpret_cast<const char *>(&size), sizeof(size));
    }

    out.close();
}

void Paint::LoadFromImageFile(std::string_view filename)
{
    auto in = std::ifstream(filename.data(), std::ios::binary);

    if (!in || !in.is_open())
        return;

    auto point_count = std::size_t{0};
    in.read(reinterpret_cast<char *>(&point_count), sizeof(point_count));

    for (std::size_t i = 0; i < point_count; ++i)
    {
        auto point = ImVec2{};
        auto color = ImColor{};
        auto size = float{};

        in.read(reinterpret_cast<char *>(&point), sizeof(point));
        in.read(reinterpret_cast<char *>(&color), sizeof(color));
        in.read(reinterpret_cast<char *>(&size), sizeof(size));

        points_.push_back(std::make_tuple(point, color, size));
    }

    in.close();
}

void Paint::ClearCanvas()
{
    points_.clear();
}
