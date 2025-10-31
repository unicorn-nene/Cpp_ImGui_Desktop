#pragma once

#include <stdint.h>
#include <string_view>
#include <vector>

#include "WindowBase.hpp"
#include <imgui.h>

/**
 * @brief 简单的 CSV 编辑窗口类,基于 ImGui 实现
 * @note 支持显示表格,修改单元格,保存/加载 CSV 文件,通过 Draw() 渲染整个窗口
 * @warning 目前只支持固定最大行列数(maxNumRows * maxNumCols)
 *
 */
class CsvEditor : public WindowBase
{
public:
    // @brief 弹窗窗口属性
    static constexpr auto s_popUpFlags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    // @brief 弹窗默认大小
    static constexpr auto s_popUpSize = ImVec2(300.0f, 100.0f);
    // @brief 弹窗默认按钮大小
    static constexpr auto s_popButtonSize = ImVec2(120.0f, 0.0f);
    // @brief 弹窗默认屏幕居中位置
    static constexpr auto s_popUpPos =
        ImVec2(1280.0f / 2.0f - s_popUpSize.x / 2.0f,
               720.0f / 2.0f - s_popUpSize.y / 2);
    // 最大行数限制
    static constexpr auto maxNumRows = 30;
    // 最大列数限制
    static constexpr auto maxNumCols = 8;

public:
    CsvEditor()
        : numCols_(0), numRows_(0), data_({}), filenameBuffer_("test.csv") {};
    virtual ~CsvEditor() {};

    void Draw(std::string_view label, bool *open = nullptr) final;

private:
    // 窗口内容绘制函数

    // @brief 绘制修改行列数的按钮
    void DrawSizeButtons();
    // @brief 绘制保存/加载 CSV 文件按钮
    void DrawIOButtons();
    // @brief 绘制表格内容,显示单元格数据
    void DrawTable();

    // @brief 绘制保存文件弹窗
    void DrawSavePopup();
    // @brief 绘制加载文件弹窗
    void DrawLoadPopup();
    // @brief 绘制单元格编辑弹窗
    void DrawvaluePopup(const int row, const int col);

    // 数据操作函数
    // @brief 保存到单元格编辑弹窗
    void SaveToCsvFile(std::string_view filename);
    // @brief 从 .csv 文件中加载数据到表格
    void LoadFromCsvFile(std::string_view filename);

    // @brief 绘制单元格值(支持格式化)
    template <typename T>
    void PlotCellValue(std::string_view formatter, const T value);
    // @brief 设置弹窗布局(位置/大小/样式)
    void SetPopupLayout();

private:
    std::int32_t numCols_{};                 // 当前列数
    std::int32_t numRows_{};                 // 当前行数
    std::vector<std::vector<float>> data_{}; // 表格数据, 按行存储
    char filenameBuffer_[256];               // 文件名输入缓冲区
};
