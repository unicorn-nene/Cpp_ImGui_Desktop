#pragma once

#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include "WindowBase.hpp"
#include <imgui.h>

/**
 * @brief 绘图窗口类(简单的画板)
 *
 */
class Paint : public WindowBase
{
public:
    /// @brief 单个绘制点的数据
    /// - ImVec2 : 点的位置（相对画布坐标）
    /// - ImColor: 点的颜色
    /// - float  : 点的半径大小
    using PointData = std::tuple<ImVec2, ImColor, float>;
    /// @brief 弹窗窗口标志（不可调整大小、不可移动、不可折叠、无滚动条）
    static constexpr auto s_popUpFlags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    /// @brief 弹窗大小（300x100 像素）
    static constexpr auto s_popUpSize = ImVec2(300.0f, 100.0f);
    /// @brief 弹窗按钮大小（120px 宽度，自适应高度）
    static constexpr auto s_popUpButtonSize = ImVec2(120.0f, 0.0f);
    /// @brief 弹窗位置（窗口居中，基于 1280x720 分辨率）
    static constexpr auto s_popUpPos =
        ImVec2(1280.0f / 2.0f - s_popUpSize.x / 2.0f,
               720.0f / 2.0f - s_popUpSize.y / 2.0f);

public:
    Paint()
        : points_({}), canvasPos_({}),
          currentDrawColor_(ImColor(255, 255, 255)), pointDrawSize_(2.0f),
          filenameBuffer_("text.bin") {};
    virtual ~Paint() {};

    /// @brief 绘制主窗口
    /// @param label 窗口标题
    /// @param open  是否打开（可选）
    void Draw(std::string_view label, bool *open = nullptr) final;

private:
    /// @brief 绘制顶部菜单栏
    void DrawMenu();
    /// @brief 绘制绘图画布
    void DrawCanvas();
    /// @brief 绘制颜色选择按钮
    void DrawColorButtons();
    /// @brief 绘制点大小选择按钮
    void DrawSizeSettings();

    /// @brief 绘制保存文件弹窗
    void DrawSavePopup();
    /// @brief 绘制加载文件弹窗
    void DrawLoadPopup();

    /// @brief 保证画布内容到文件中
    /// @param filename 目标文件名
    void SaveToImageFile(std::string_view filename);
    /// @brief 从文件中加载画布内容
    /// @param filename 源文件名字
    void LoadFromImageFile(std::string_view filename);
    /// @brief 清空画布内容(清理points容器)
    void ClearCanvas();

private:
    std::uint32_t numRows_ = 800;   // 画布高度
    std::uint32_t numCols_ = 600;   // 画布宽度
    std::uint32_t numChannels_ = 3; // 颜色通道数量(默认3通道RGB)
    // 画布尺寸
    ImVec2 canvasSize_ =
        ImVec2(static_cast<float>(numRows_), static_cast<float>(numCols_));

    std::vector<PointData> points_{}; // 已经绘制点的集合
    ImVec2 canvasPos_{};              // 画布左上角的位置(相对于窗口)

    ImColor currentDrawColor_{}; // 当前绘制点的颜色
    float pointDrawSize_{};      // 当前绘制点 的大小(半径)

    char filenameBuffer_[256]; // 文件名缓冲区
};
