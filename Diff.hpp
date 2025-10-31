#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <imgui.h>

#include "WindowBase.hpp"

// @brief 简单文本差异查看器（Diff Viewer）
// 功能：
// - 加载两个文本文件，比较其内容差异。
// - 显示两个文本文件的差异视图，并提供统计信息。
// - 支持通过 UI 选择文件和查看差异。
// 设计思路：
// - 使用 FileContent 保存文件内容，每行作为一个 std::string。
// - diffResult1 / diffResult2 保存差异对齐后的文件行，便于显示差异。
// - Draw() 统一渲染窗口，通过子函数分工实现不同区域。
class DiffViewer : public WindowBase
{
public:
    // @brief 文件内容类型, 每行一个 std::string
    using FileContent = std::vector<std::string>;

public:
    DiffViewer()
        : filePath1_("text1.txt"), filePath2_("text2.txt"), fileContent1_({}),
          fileContent2_({}), diffResult1_({}), diffResult2_({}) {};

    virtual ~DiffViewer();

    /**
     * @brief 绘制 DiffViewer 窗口
     *
     * @param label 窗口标题
     * @param open  控制窗口显示
     */
    void Draw(std::string_view label, bool *open = nullptr) final;

private:
    /* UI 绘制函数*/
    // @brief 绘制文件选择区域
    void DrawSelection();
    // @brief 绘制差异对比视图
    void DrawDiffView();
    // @brief 绘制差异统计信息
    void DrawStats();

    /* 文件与差异处理 */

    /**
     * @brief 加载指定路径的文件内容
     *
     * @param file_path 要加载的文件内容
     * @return FileContent
     */
    FileContent LoadFileContent(std::string_view file_path);
    /**
     * @brief 保存内容到文件
     *
     * @param file_path 要保存的文件路径
     * @param file_content 要保存的文件内容
     */
    void SaveFileContent(std::string_view file_path, FileContent &file_content);
    // @brief 计算两个文件的差异,生成 diffResult1 / diffResult2
    void CreateDiff();

private:
    /* 文件路径 */
    std::string filePath1_{}; // 第一个文件路径
    std::string filePath2_{}; // 第二个文件路径

    /* 原始文件内容 */
    FileContent fileContent1_{}; // 第一个文件内容
    FileContent fileContent2_{}; // 第二个文件内容

    /* 差异结果 */
    FileContent diffResult1_{}; // 第一个文件的差异显示内容
    FileContent diffResult2_{}; // 第二个文件的差异显示内容
};
