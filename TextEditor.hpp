#pragma once

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>

#include "WindowBase.hpp"

namespace fs = std::filesystem;

/// @brief 文本编辑器窗口类，继承自 WindowBase
/// 提供基本的文本编辑功能，包括加载、保存、编辑和显示文件信息
class TextEditor : public WindowBase
{
public:
    /// @brief 文本缓冲区大小(最大可编辑字符数)
    static constexpr auto s_bufferSize = std::size_t{1024};
    /// @brief  弹窗窗口的通用配置（不可调整大小、不可移动、不可折叠、无滚动条）
    static constexpr auto s_popUpFlags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    /// @brief 弹窗窗口的固定大小
    static constexpr auto s_popUpSize = ImVec2(300.0F, 100.0F);
    /// @brief 弹窗中按钮的统一尺寸
    static constexpr auto s_popUpButtonSize = ImVec2(120.0F, 0.0F);
    /// @brief 弹窗在屏幕中的位置
    static constexpr auto s_popUpPos =
        ImVec2(1280.0F / 2.0F - s_popUpSize.x / 2.0F,
               720.0F / 2.0F - s_popUpSize.y / 2.0F);

public:
    TextEditor() : currentFilename_({})
    {
        std::memset(textBuffer_, 0, s_bufferSize);
    }
    virtual ~TextEditor() {};

    void Draw(std::string_view label, bool *open = nullptr) final;

private:
    /// @brief 绘制菜单栏
    void DrawMenu();
    /// @brief 绘制保存文件的弹窗
    void DrawSavePopup();
    /// @brief 绘制加载文件的弹窗
    void DrawLoadPopup();
    /// @brief 绘制文本编辑区域
    void DrawContent();
    /// @brief 绘制文件信息区域(文本输入框)
    void DrawInfo();

    /// @brief 将文本缓冲区保存到文件
    /// @param filename 要保存的文件名
    void SaveToFile(std::string_view filename);
    /// @brief 从文件加载内容到缓冲区
    /// @param filename 加载的文件名
    void LoadFromFile(std::string_view filename);

    /// @brief  获取文件的扩展名（例如 ".txt"）
    /// @param filename 文件名
    /// @return 文件扩展名
    std::string GetFileExtension(std::string_view filename);

private:
    // 文本编辑缓冲区（存储当前正在编辑的文本内容）
    char textBuffer_[s_bufferSize];
    std::string currentFilename_{}; // 当前打开/保存的文件名
};
