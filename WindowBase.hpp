#pragma once

#include <filesystem>
#include <string_view>

#include <imgui.h>

#include <SimpleIni.h>
#include <implot.h>

class WindowBase
{
public:
    // 主窗口标志常量:禁止调整大小, 移动, 折叠和滚动条
    constexpr static auto s_mainWindowFlags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    // 主窗口默认尺寸常量
    constexpr static auto s_mainWindowSize = ImVec2(1280.0f, 720.0f);
    // 主窗口默认位置常量(左上角)
    constexpr static auto s_mainWindowPos = ImVec2(0.0f, 0.0f);

public:
    WindowBase() : ini_(CSimpleIniA()) {};
    virtual ~WindowBase()
    {
    }

    void SettingsMenuBar();
    void LoadTheme();
    void SaveTheme();

    /// @brief 纯虚函数:绘制整个文件资源管理器窗口
    /// @param label 窗口标题
    /// @param open  是否打开窗口的标志
    virtual void Draw(std::string_view label, bool *open = nullptr) = 0;

protected:
    void DrawColorsSettings(bool *open = nullptr);
    /// @brief 静态方法：获取默认的颜色样式配置
    static ImGuiStyle defaultColorStyle();

protected:
    // INI配置文件对象：用于读写应用程序设置和主题配置
    CSimpleIniA ini_;
};
