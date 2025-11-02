#pragma once
#include <filesystem>
#include <imgui.h>
#include <string_view>

#include "WindowBase.hpp"

namespace fs = std::filesystem;

/**
 * @brief 文件资源管理器窗口类,继承 windowBase
 *        提供文件浏览, 筛选,操作(打开,重命名,删除)等功能
 */
class FileExplorer : public WindowBase
{
public:
    FileExplorer()
        : currentPath_(fs::current_path()), selectedEntry_(fs::path()) {};
    virtual ~FileExplorer();

    void Draw(std::string_view label, bool *open = nullptr) final;

private:
    // @brief 绘制菜单栏
    void DrawMenu();
    // @brief 绘制目录文件
    void DrawContent();
    // @brief 绘制文件操作按钮
    void DrawActions();
    // @brief 绘制文件筛选控件
    void DrawFilter();

    // @biref 使用系统默认程序打开文件
    void openFileWithDefaultEditor();
    // @brief 弹出重命名确认对话框
    void renameFilePopup();
    // @brief 弹出删除确认对话框
    void deleteFilePopup();

    // @brief 执行文件重命名操作
    bool renameFile(const fs::path &old_path, const fs::path &new_path);
    // @brief 执行文件删除操作
    bool deleteFile(const fs::path &path);

private:
    fs::path currentPath_;   //当前浏览的目录路径
    fs::path selectedEntry_; // 当前选中的文件或目录

    bool renameDialogOpen_ = false; // 是否显示重命名对话框
    bool deleteDialogOpen_ = false; // 是否显示删除确认对话框
};
