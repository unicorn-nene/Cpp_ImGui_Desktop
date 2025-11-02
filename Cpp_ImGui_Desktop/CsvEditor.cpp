#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "CsvEditor.hpp"

void CsvEditor::Draw(std::string_view label, bool *open)
{
    ImGui::SetNextWindowPos(s_mainWindowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(s_mainWindowSize, ImGuiCond_Always);

    ImGui::Begin(label.data(), open, s_mainWindowFlags);

    DrawSizeButtons();

    ImGui::Separator();
    DrawIOButtons();

    ImGui::Separator();
    DrawTable();

    ImGui::End();
}

// @brief 绘制修改表格行列数的按钮和滑块
// 功能：
// 1. 用户可以通过 SliderInt 或按钮调整行数和列数。
// 2. 根据用户操作，动态增加或删除表格数据行/列。
// 3. 数据保持初始化为 0.0F。
//
// @note
// - 使用 bool 标志追踪用户操作类型（增加/删除行列）。
// - 根据标志对 data 容器进行增删操作。
//
// @warning
// - 行列数受 maxNumRows / maxNumCols 限制，避免越界。
// - 增加行时，要创建与当前列数一致的 vector。
// - 增加列时，要为每一行增加新元素。
// - 删除行/列时，直接 pop_back。
// - 逻辑分支按“增加行、增加列、删除行、删除列”顺序处理，避免冲突。
void CsvEditor::DrawSizeButtons()
{
    // 用户操作标志
    auto user_added_rows = false;
    auto user_dropped_rows = false;
    auto user_added_cols = false;
    auto user_dropped_cols = false;

    // 当前滑块值
    auto slider_value_rows = numRows_;
    auto slider_value_cols = numCols_;

    // 绘制行控制
    ImGui::Text("Num Rows");
    ImGui::SameLine();
    if (ImGui::SliderInt("##numRows", &slider_value_rows, 0, maxNumRows))
    {
        user_added_rows = slider_value_rows > numRows_ ? true : false;
        user_dropped_rows = !user_added_rows;

        numRows_ = slider_value_rows;
    }

    ImGui::SameLine();
    if (ImGui::Button("Add Row") && numRows_ < maxNumRows)
    {
        ++numRows_;
        user_added_rows = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Drop Row") && numRows_ > 0)
    {
        --numRows_;
        user_dropped_rows = true;
    }

    // 绘制列控制
    ImGui::Text("Num Cols: ");
    ImGui::SameLine();
    if (ImGui::SliderInt("##numCols", &slider_value_cols, 0, maxNumCols))
    {
        user_added_cols = slider_value_cols > numCols_ ? true : false;
        user_dropped_cols = !user_added_cols;

        numCols_ = slider_value_cols;
    }

    ImGui::SameLine();
    if (ImGui::Button("Add Col") && numCols_ < maxNumCols)
    {
        ++numCols_;
        user_added_cols = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Drop Col") && numCols_ > 0)
    {
        --numCols_;
        user_dropped_cols = true;
    }

    // 同步 data 容器, 当前的表格行数
    const auto num_rows_i32 = static_cast<std::int32_t>(data_.size());

    // 增加行时,在 data 末尾(表格的最下面)添加新行,每行列数与当前 numCols 一致
    if (user_added_rows)
    {
        for (auto row = num_rows_i32; row < numRows_; ++row)
        {
            data_.push_back(std::vector<float>(numRows_, 0.0f));
        }
    }
    // 增加一列, 对每行末尾(表格最右边)添加一个元素
    else if (user_added_cols)
    {
        for (std::int32_t row = 0; row < numRows_; ++row)
        {
            const auto num_cols_i32 = static_cast<int32_t>(data_[row].size());
            for (auto col = num_cols_i32; col < numCols_; ++col)
            {
                data_[row].push_back(0.0f);
            }
        }
    }
    // 删除一行,从末尾(表格最下面)添加一行元素
    else if (user_dropped_rows)
    {
        for (auto row = num_rows_i32; row > numRows_; --row)
        {
            data_.pop_back();
        }
    }
    // 删除一列,在每行末尾(最右侧)删除一个元素
    else if (user_dropped_cols)
    {
        for (std::int32_t row = 0; row < numRows_; ++row)
        {
            const auto num_cols_i32 = static_cast<int32_t>(data_[row].size());
            for (auto col = num_cols_i32; col > numCols_; --col)
            {
                data_[row].pop_back();
            }
        }
    }
}

// @brief
// 绘制 CSV 编辑器的文件操作按钮
// 功能：提供保存、加载和清空表格的操作
//
// @note
// - 使用弹窗处理文件保存/加载，避免直接操作文件时阻塞界面。
// - 清空操作直接重置 data 和行列数。
//
// @warning
// - Save / Load 按钮会打开对应的 ImGui 弹窗，由 DrawSavePopup / DrawLoadPopup 实现
// - Clear 按钮会完全清空表格数据
void CsvEditor::DrawIOButtons()
{
    // 保存按钮
    if (ImGui::Button("Save"))
        ImGui::OpenPopup("Save File");

    // 加载按钮
    ImGui::SameLine();
    if (ImGui::Button("Load"))
        ImGui::OpenPopup("Load File");

    // 清空按钮
    ImGui::SameLine();
    if (ImGui::Button("Clear"))
    {
        data_.clear();
        numRows_ = 0;
        numCols_ = 0;
    }

    // 绘制弹窗（如果已打开）
    DrawSavePopup();
    DrawLoadPopup();
}

// @brief 绘制 CSV 表格内容
// 功能：显示当前数据表格，支持单元格点击编辑和鼠标悬停提示
// @note
// - 使用 ImGui Table 绘制表格，列数动态变化
// - 表头用字母 A, B, C... 表示
// - 单元格点击时打开编辑弹窗
// - 鼠标悬停时显示单元格坐标
// @warning
// - 如果 numCols 为 0，则直接返回，不绘制表格
// - row_clicked / col_clicked 用于记录点击的单元格，传给 DrawValuePopup
void CsvEditor::DrawTable()
{
    constexpr static auto table_flags =
        ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuter;

    // 记录点击的单元格坐标
    static auto row_clicked = 0;
    static auto col_clicked = 0;

    if (numCols_ == 0)
        return;

    // 开始绘制表格
    ImGui::BeginTable("Table", numCols_, table_flags);

    // 设置列宽和列名（A, B, C...）
    for (std::int32_t col = 0; col < numCols_; ++col)
    {
        const auto col_name = fmt::format("{}", 'A' + col);
        ImGui::TableSetupColumn(col_name.data(),
                                ImGuiTableColumnFlags_WidthFixed,
                                1280.0f / static_cast<float>(numCols_));
    }

    //绘制表头
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    for (std::int32_t col = 0; col < numCols_; ++col)
        PlotCellValue("%c", 'A' + col);

    // 绘制表格单元格
    for (std::int32_t row = 0; row < numRows_; ++row)
    {
        for (std::int32_t col = 0; col < numCols_; ++col)
        {
            // 绘制单元格数据
            PlotCellValue("%f", data_[row][col]);

            // 点击单元格弹出编辑框
            if (ImGui::IsItemClicked())
            {
                ImGui::OpenPopup("Change Value");
                row_clicked = row;
                col_clicked = col;
            }
            // 悬停显示信息
            else if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Cell: (%d, %d)", row, col);
            }
        }

        ImGui::TableNextRow();
    }

    // 绘制单元格编辑弹窗
    DrawvaluePopup(row_clicked, col_clicked);

    ImGui::EndTable();
}
