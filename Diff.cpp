#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <implot.h>

#include "Diff.hpp"

// @brief 绘制 DiffViewer 主窗口
// 功能：
// 1. 设置窗口位置和大小（mainWindowPos / mainWindowSize），固定布局。
// 2. 使用 ImGui::Begin / End 包裹窗口内容。
// 3. 调用子函数绘制不同区域：文件选择、差异视图、统计信息。
// @note
// - 窗口布局固定，避免用户拖动和缩放影响差异视图显示。
// - 子函数分工明确，UI 与逻辑相对独立，方便维护。
// @warning
// - 需要保证 DrawSelection() 在 DrawDiffView() 之前执行，以便更新 diff 数据。
void DiffViewer::Draw(std::string_view label, bool *open)
{
    ImGui::SetNextWindowPos(s_mainWindowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(s_mainWindowSize, ImGuiCond_Always);

    ImGui::Begin(label.data(), open, s_mainWindowFlags);

    DrawSelection();
    DrawDiffView();
    DrawStats();

    ImGui::End();
}

// @brief 绘制文件选择和操作区域
// 功能：
// 1. 提供左/右文件路径输入框。
// 2. 提供保存按钮，可将当前内容保存到对应文件。
// 3. 提供 Compare 按钮，加载文件内容并生成差异。
// @note
// - 使用 ImGui::InputText 让用户输入文件路径或编辑路径。
// - 保存按钮直接调用 SaveFileContent() 保存当前内存数据。
// - Compare 按钮先加载文件内容，再调用 CreateDiff() 生成差异结果。
// @warning
// - 保存按钮使用 "###Left" / "###Right" 避免与 InputText 重复 ID，保证 ImGui 唯一性。
// - Compare 操作会覆盖 fileContent1 / fileContent2，之后更新 diffResult1 / diffResult2。
// - 必须在 Compare 之后再绘制差异视图，确保数据同步。
void DiffViewer::DrawSelection()
{
    ImGui::InputText("Left", &filePath1_);
    ImGui::SameLine();
    if (ImGui::Button("Save###Left"))
    {
        SaveFileContent(filePath1_, fileContent1_);
    }

    ImGui::InputText("Right", &filePath2_);
    ImGui::SameLine();
    if (ImGui::Button("Save###Right"))
    {
        SaveFileContent(filePath2_, fileContent2_);
    }

    if (ImGui::Button("Compare"))
    {
        fileContent1_ = LoadFileContent(filePath1_);
        fileContent2_ = LoadFileContent(filePath2_);

        CreateDiff();
    }
}

void DiffViewer::DrawDiffView()
{
    // 中间 swap 区域宽度
    constexpr static auto swap_width = 40.0f;
    // 父窗口大小
    const auto parent_size = ImVec2(ImGui::GetContentRegionAvail().x, 500.0f);
    // 左右文件窗口大小
    const auto child_size =
        ImVec2(parent_size.x / 2.0f - swap_width, parent_size.y);
    // 中间 swap 区域大小
    const auto swap_size = ImVec2(swap_width, child_size.y);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::BeginChild("Parent", parent_size, true);

    // 左侧文件内容
    if (ImGui::BeginChild("Left", child_size, false))
    {
        for (std::size_t i = 0; i < fileContent1_.size(); ++i)
        {
            if (!diffResult1_[i].empty())
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                                   "%s",
                                   fileContent1_[i].data());
            }
            else
                ImGui::Text("%s", fileContent1_[i].data());
        }

        ImGui::EndChild(); // end Left
    }

    // 中间交换区域
    const auto line_height = ImGui::GetTextLineHeightWithSpacing();
    const auto button_size = ImVec2(15.0f, line_height);

    if (ImGui::BeginChild("Swap", swap_size, true))
    {
        for (std::size_t i = 0; i < diffResult1_.size(); i++)
        {
            const auto left_label =
                fmt::format("<##{}", i); // 左侧行向右侧行同步
            const auto right_label =
                fmt::format(">##{}", i); // 右侧行向左侧行同步

            // 将 右侧行同步到左侧
            if (!diffResult1_[i].empty() || !diffResult2_[i].empty())
            {
                if (ImGui::Button(left_label.data(), button_size))
                {
                    if (fileContent1_.size() > i && fileContent2_.size() > i)
                        fileContent1_[i] = fileContent2_[i];
                    else if (fileContent2_.size() > i)
                        fileContent1_.push_back(fileContent2_[i]);

                    // 同步后更新差异
                    CreateDiff();
                }


                ImGui::SameLine();
                // 将左侧同步到右侧
                if (ImGui::Button(right_label.data(), button_size))
                {
                    if (fileContent1_.size() > i && fileContent2_.size() > i)
                        fileContent2_[i] = fileContent2_[i];
                    else if (fileContent1_.size() > i)
                        fileContent2_.push_back(fileContent1_[i]);

                    CreateDiff();
                }
            }
            else
            {
                // 当前行无差异时，跳过按钮，增加行高保持对齐
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + line_height);
            }
        }
    }
    ImGui::EndChild(); // end swap

    ImGui::SameLine();
    // 右侧文件内容
    if (ImGui::BeginChild("Diff2", child_size, false))
    {
        for (std::size_t i = 0; i < fileContent2_.size(); ++i)
        {
            if (!diffResult2_[i].empty())
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                                   "%s",
                                   fileContent2_[i].data());
            }
            else
            {
                ImGui::Text("%s", fileContent2_[i].data());
            }
        }

        ImGui::EndChild(); // end Diff2
    }

    ImGui::EndChild(); // end parent

    ImGui::PopStyleVar();
}

// @brief 绘制统计信息
// 功能：
// - 计算左侧文件中存在差异的行数（通过 diffResult1 判断）。
// - 在窗口底部显示差异行的数量。
// @note
// - 遍历 diffResult1，统计非空字符串（表示该行存在差异）。
// - 使用 SetCursorPosY 定位到窗口底部，再绘制统计文本。
// @warning
// - 这里只统计了左文件的差异行数，但由于 diffResult1/2 是对应的，统计结果等价。
// - 窗口底部固定 20.0F 的偏移可能在不同 DPI 下需要调整。
void DiffViewer::DrawStats()
{
    auto diff_line_count = std::uint32_t{0};
    for (const auto &line : diffResult1_)
    {
        if (!line.empty())
            ++diff_line_count;
    }

    // 定位到窗口底部，再绘制统计文本
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 20.0f);
    ImGui::Text("Diff lines count: %u", diff_line_count);
}


// @brief 从文件加载内容
// @param file_path 文件路径
// @return 文件内容（逐行存入 vector<string>）
// @note
// - 打开文件并按行读取，将每行保存到 FileContent。
// - 如果文件不存在或无法打开，返回空容器。
// @warning
// - 使用 std::getline 逐行读取，自动去掉换行符。
// - 失败时返回空 vector，不会抛出异常。
// - 支持 UTF-8 文本文件，但不处理 BOM/编码问题。
DiffViewer::FileContent DiffViewer::LoadFileContent(std::string_view file_path)
{
    auto content = FileContent{};
    auto in = std::ifstream{file_path.data()};

    if (in.is_open())
    {
        auto line = std::string{};
        while (std::getline(in, line))
            content.push_back(line);
    }

    in.close();
    return content;
}


void DiffViewer::SaveFileContent(std::string_view file_path,
                                 FileContent &file_content)
{
    auto out = std::ofstream{file_path.data()};

    if (out.is_open())
    {
        for (const auto &line : file_content)
            out << line << '\n';
    }

    out.close();
}


// @brief 生成文件差异
// 功能：
// - 比较 fileContent1 和 fileContent2 的每一行，生成差异结果。
// - 如果对应行不同，则记录原始内容到 diffResult1/2。
// - 如果相同，则在 diffResult1/2 存放空字符串。
// @note
// - 遍历两文件最大行数，缺失的行用 "EMPTY" 代替。
// - 结果保证 diffResult1/2 与最大行数相同，方便 UI 对齐。
// @warning
// - "EMPTY" 字符串用于标记文件缺行，可能在 UI 上显示出来（可改为特殊符号或高亮）。
// - diffResult1/2 与 fileContent1/2 行数一致，方便渲染时直接索引。
void DiffViewer::CreateDiff()
{
    diffResult1_.clear();
    diffResult2_.clear();

    const auto max_num_lines =
        std::max(fileContent1_.size(), fileContent2_.size());

    for (std::size_t i = 0; i < max_num_lines; ++i)
    {
        const auto line1 =
            i < fileContent1_.size() ? fileContent1_[i] : "EMPTY";
        const auto line2 =
            i < fileContent2_.size() ? fileContent2_[i] : "EMPTY";

        if (line1 != line2)
        {
            diffResult1_.push_back(fileContent1_[i]);
            diffResult2_.push_back(fileContent2_[i]);
        }
        else
        {
            diffResult1_.push_back("");
            diffResult2_.push_back("");
        }
    }
}
