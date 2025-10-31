#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "FileExplorer.hpp"
/**
 * @brief 绘制整个文件资源管理器窗口
 *
 *      窗口分为四个区域 -> 菜单栏,文件内容,操作区,过滤器
 * @param label 窗口标题
 * @param open  是否打开窗口的标志
 */
void FileExplorer::Draw(std::string_view label, bool *open)
{
    ImGui::SetNextWindowSize(s_mainWindowSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(s_mainWindowPos, ImGuiCond_Always);

    ImGui::Begin(label.data(), open, s_mainWindowFlags);

    DrawMenu();

    ImGui::Separator();
    DrawContent();

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 100.0f);
    ImGui::Separator();
    DrawActions();

    ImGui::Separator();
    DrawFilter();

    ImGui::End();
}

/**
 * @brief 绘制菜单栏
 *
 * 实现: 提供一个 "Go Up"按钮,点击可以返回上级目录,并显示当前路径.
 */
void FileExplorer::DrawMenu()
{
    if (ImGui::Button("Go Up"))
    {
        if (currentPath_.has_parent_path())
        {
            currentPath_ = currentPath_.parent_path();
        }
    }

    ImGui::SameLine();

    //  显示当前目录的绝对路径
    ImGui::Text("Current directory: %s", currentPath_.string().c_str());
}

/**
 * @brief 绘制目录内容
 *
 * 实现: 使用 std::filesystem::directory_iterator 遍历当前目录
 *       将文件夹标记为[D], 文件标记为[F],并允许点击选择.
 *
 * 注意: 目录点击后进入下一级(currentPath更新)
 *      文件点击后只是选中,等待后续操作
 */
void FileExplorer::DrawContent()
{
    for (const auto &entry : fs::directory_iterator(currentPath_))
    {
        const auto is_selected = (entry.path() == selectedEntry_);
        const auto is_directory = entry.is_directory();
        const auto is_file = entry.is_regular_file();
        auto entry_name = entry.path().filename().string();

        if (is_directory)
            entry_name = "[D] " + entry_name;
        if (is_file)
            entry_name = "[F] " + entry_name;

        if (ImGui ::Selectable(entry_name.data(), is_selected))
        {
            if (is_directory)
                currentPath_ /= entry.path().filename();

            selectedEntry_ = entry.path();
        }
    }
}

/**
 * @brief 绘制操作按钮(打开,重命名,删除)
 *
 * 实现思路: 先显示选中的文件/目录信息,然后根据类型显示不同操作.
 *
 * 注意事项:
 * 非法选中(比如空选项)需要禁用按钮;
 * 弹窗通过 ImGui::OpenPopup 打开,再由 renameFilePopup() / deleteFilePopup() 绘制
 *
 */
void FileExplorer::DrawActions()
{
    if (fs::is_directory(selectedEntry_))
    {
        ImGui::Text("Selected dir: %s", selectedEntry_.string().c_str());
    }
    else if (fs::is_regular_file(selectedEntry_))
    {
        ImGui::Text("Selected file: %s", selectedEntry_.string().c_str());
    }
    else
    {
        // 如果没有选中任何文件,则显示不可用按钮
        ImGui::Text("Selected file: n/a");
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
                            ImGui::GetStyle().Alpha * 0.0f);
        ImGui::Button("Non-Clickable button");
        ImGui::PopStyleVar();
        return;
    }
}

/**
 * @brief 文件筛选器(按扩展名过滤统计)
 *
 * 实现思路: 用户输入扩展名(如".txt"),统计当前目录下符合的文件数
 *
 * 注意事项:
 * - 输入为空时直接返回;
 * - 只对 regular_file 文件进行统计
 *
 */
void FileExplorer::DrawFilter()
{
    static char extension_filter[16] = {'/0'};

    ImGui::Text("Filter by extension");
    ImGui::SameLine();
    ImGui::InputText("##inFilter", extension_filter, sizeof(extension_filter));

    if (std::strlen(extension_filter) == 0)
        return;

    auto filter_file_count = std::uint32_t{0};
    for (const auto &entry : fs::directory_iterator(currentPath_))
    {
        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension().string() == extension_filter)
            ++filter_file_count;
    }

    ImGui::Text("Number of files : %u", filter_file_count);
}


/**
 *  @brief 使用系统默认程序打开文件
 *
 *  思路：根据不同平台调用系统命令。
 * - Windows: start;
 * - macOS:  open;
 * - Linux:  xdg-open;
 *
 */
void FileExplorer::openFileWithDefaultEditor()
{
#ifdef _WIN32
    const auto command = "start \"\" \"" + selectedEntry_.string() + "\"";
#elif __APPLE_
    const auto command = "open \"" + selectedEntry_.string() + "\"";
#else
    const auto command = "xdg=open \"" + selectedEntry_string() + "\"";
#endif
}


/**
 * @brief 绘制重命名操作弹窗
 *
 * 实现思路:弹出模态对话框,输入新文件名字,执行文件名
 *
 * 注意事项:
 * - 使用静态 buffer 存储新名字,避免作用域问题;
 * - 重命名成功后更新 selectedEntry,并清空输入
 *
 */
void FileExplorer::renameFilePopup()
{
    if (ImGui::BeginPopupModal("Rename File", &renameDialogOpen_))
    {
        static char buffer_name[512] = {'\0'};
        ImGui::Text("New name: ");
        ImGui::InputText("###newName", buffer_name, sizeof(buffer_name));

        if (ImGui::Button("rename"))
        {
            auto new_path = selectedEntry_.parent_path() / buffer_name;
            if (renameFile(selectedEntry_, new_path))
            {
                renameDialogOpen_ = false;
                selectedEntry_ = new_path;
                std::memset(buffer_name, 0, sizeof(buffer_name));
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            renameDialogOpen_ = false;
        }

        ImGui::EndPopup();
    }
}

/**
 * @brief 绘制删除弹窗
 *
 * 实现思路: 模态对话框确认删除,避免误操作
 *
 * 注意事项:
 * - 删除成功后清空 selectedEntry.
 * - 删除失败会在 deleteFile() 函数中打印错误日志
 */
void FileExplorer::deleteFilePopup()
{
    if (ImGui::BeginPopupModal("Delete File", &deleteDialogOpen_))
    {
        ImGui::Text("Are you sure you want to delete %s?",
                    selectedEntry_.filename().string().c_str());

        if (ImGui::Button("Yes"))
        {
            if (deleteFile(selectedEntry_))
            {
                selectedEntry_.clear();
            }
            deleteDialogOpen_ = false;
        }

        ImGui::SameLine();
        if (ImGui::Button("No"))
        {
            deleteDialogOpen_ = false;
        }

        ImGui::EndPopup();
    }
}

// @brief 执行文件重命名操作
// @param old_path 原文件路径
// @param new_path 新文件路径
//
//  注意事项：
// - 使用 try/catch 捕获异常，避免程序崩溃。
bool FileExplorer::renameFile(const fs::path &old_path,
                              const fs::path &new_path)
{
    try
    {
        fs::rename(old_path, new_path);
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
}

// @brief 执行文件删除操作
// @param path 文件路径
//
//  注意事项：
// - 使用 try/catch 捕获异常，避免程序崩溃。
bool FileExplorer::deleteFile(const fs::path &path)
{
    try
    {
        fs::remove(path);
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
