#include <filesystem>
#include <fstream>
#include <iostream>

#include <SimpleIni.h>
#include <imgui.h>
#include <implot.h>

#include "WindowBase.hpp"

// 匿名命名空间工具函数
namespace
{
/**
     * @brief 从INI配置文件中加载颜色值(ImU32格式)
     *
     * @param ini_ 指向 CSimpleIniA配置对象的指针,用于读取配置
     * @param section INI文件中的节名称,如"[Colors]"
     * @param key  配置项键名,如"BackgroundColor"
     * @param def 默认颜色值,当配置项不存在时返回此值
     * @return ImU32 解析后的颜色值(ImU32格式)，如果配置项不存在则返回默认值
     */
ImU32 LoadColor(CSimpleIniA *ini_,
                const char *section,
                const char *key,
                ImU32 def)
{
    // 从 ini_ 文件中读取颜色配置值(十六进制字符串)
    const char *wc = ini_->GetValue(section, key, nullptr);

    // 不存在,则使用默认颜色
    if (!wc)
        return def;

    // 将十六进制字符串转换为整数
    auto value = std::strtoul(wc, nullptr, 16);
    return static_cast<ImU32>(value);
}

/**
 * @brief 将ImU32格式颜色值保存到INI配置文件中
 *
 * @param ini_     指向CSimpleIniA配置对象的指针，用于写入配置
 * @param section INI文件中的节名称，如"[Colors]"
 * @param key     配置项键名，如"BackgroundColor"
 * @param val     要保存的颜色值(ImU32格式)
 */
void SaveColor(CSimpleIniA *ini_,
               const char *section,
               const char *key,
               ImU32 val)
{
    // 将颜色整数(U32)转为十六进制字符串形式写入配置文件
    char buf[2 * 4 + 1] = {'\0'}; // 8字符十六进制数 + 终止符
    snprintf(buf, sizeof(buf), "%x", val);
    ini_->SetValue(section, key, buf);
}
} // namespace

/**
 * @brief 显示设置菜单栏
 *
 * */
void WindowBase::SettingsMenuBar()
{
    static auto theme_menu_open = false;
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Settings"))
        {
            //菜单项"Theme", 点击后打开主题设置窗口
            ImGui::MenuItem("Theme", nullptr, &theme_menu_open);
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // 如果用户打开了 Theme 菜单,则绘制对应窗口
    if (theme_menu_open)
        DrawColorsSettings(&theme_menu_open);
}

/// @brief 将 ini_ 文件加载主题设置
void WindowBase::LoadTheme()
{
#ifdef _DEBUG
    std::cout << "Loading theme!\n";
#endif

    const char ini_filepath[] = "test.ini_";
    const char section[] = "theme";

    // 若配置文件不存在
    if (std::filesystem::exists(ini_filepath))
    {
        ImGui::GetStyle() = defaultColorStyle();
        std::ofstream{ini_filepath}.close();
        return;
    }

    // 加载 ini_ 文件
    ini_.LoadFile(ini_filepath);

    auto &style = ImGui::GetStyle();

    // 从 ini_ 文件读取各种样式参数,若无则保留原值

    style.WindowPadding.x = (float)ini_.GetDoubleValue(section,
                                                       "WindowPaddingX",
                                                       style.WindowPadding.x);
    style.WindowPadding.y = (float)ini_.GetDoubleValue(section,
                                                       "WindowPaddingY",
                                                       style.WindowPadding.y);
    style.WindowRounding = (float)ini_.GetDoubleValue(section,
                                                      "WindowRounding",
                                                      style.WindowRounding);

    style.FramePadding.x = (float)ini_.GetDoubleValue(section,
                                                      "FramePaddingX",
                                                      style.FramePadding.x);
    style.FramePadding.y = (float)ini_.GetDoubleValue(section,
                                                      "FramePaddingY",
                                                      style.FramePadding.y);
    style.FrameRounding = (float)ini_.GetDoubleValue(section,
                                                     "FrameRounding",
                                                     style.FrameRounding);

    style.ItemSpacing.x = (float)ini_.GetDoubleValue(section,
                                                     "ItemSpacingX",
                                                     style.ItemSpacing.x);
    style.ItemSpacing.y = (float)ini_.GetDoubleValue(section,
                                                     "ItemSpacingY",
                                                     style.ItemSpacing.y);

    style.ItemInnerSpacing.x =
        (float)ini_.GetDoubleValue(section,
                                   "ItemInnerSpacingX",
                                   style.ItemInnerSpacing.x);
    style.ItemInnerSpacing.y =
        (float)ini_.GetDoubleValue(section,
                                   "ItemInnerSpacingY",
                                   style.ItemInnerSpacing.y);

    style.IndentSpacing = (float)ini_.GetDoubleValue(section,
                                                     "IndentSpacing",
                                                     style.IndentSpacing);
    style.ScrollbarSize = (float)ini_.GetDoubleValue(section,
                                                     "ScrollbarSize",
                                                     style.ScrollbarSize);
    style.ScrollbarRounding =
        (float)ini_.GetDoubleValue(section,
                                   "ScrollbarRouning",
                                   style.ScrollbarRounding);

    style.GrabMinSize =
        (float)ini_.GetDoubleValue(section, "GrabMinSize", style.GrabMinSize);
    style.GrabRounding =
        (float)ini_.GetDoubleValue(section, "GrabRounding", style.GrabRounding);
    style.WindowTitleAlign.x =
        (float)ini_.GetDoubleValue(section,
                                   "WindowTitleAlignX",
                                   style.WindowTitleAlign.x);
    style.WindowTitleAlign.y =
        (float)ini_.GetDoubleValue(section,
                                   "WindowTileAlignY",
                                   style.WindowTitleAlign.y);

    style.ButtonTextAlign.x =
        (float)ini_.GetDoubleValue(section,
                                   "ButtonTextAlign",
                                   style.ButtonTextAlign.x);
    style.ButtonTextAlign.y =
        (float)ini_.GetDoubleValue(section,
                                   "ButtonTextAlign",
                                   style.ButtonTextAlign.y);

    // 读取每个颜色项, 用户的自定义颜色项
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
    {
        const char *name = ImGui::GetStyleColorName(i);
        const auto default_color =
            ImGui::ColorConvertFloat4ToU32(style.Colors[i]);
        const auto color_u32 = LoadColor(&ini_, section, name, default_color);
        style.Colors[i] = ImGui::ColorConvertU32ToFloat4(color_u32);
    }
}

/**
 * @brief 将当前主题设置保存到 ini_ 文件中
 *
 */
void WindowBase::SaveTheme()
{
#ifdef _DEBUG
    std::cout << "Saving theme!\n";
#endif

    const char ini_filepath[] = "test.ini_";
    const char section[] = "theme";

    if (!ImGui::GetCurrentContext())
        return;

    // 尝试加载失败,则直接返回
    SI_Error rc = ini_.LoadFile(ini_filepath);
    if (rc < 0)
        return;

    auto &style = ImGui::GetStyle();

    // 写入各个样式参数(浮点类型)  ini_.SetDoubleValue(section, "GlobalAlpha", style.Alpha);
    ini_.SetDoubleValue(section, "WindowPaddingX", style.WindowPadding.x);
    ini_.SetDoubleValue(section, "WindowPaddingY", style.WindowPadding.y);
    ini_.SetDoubleValue(section, "WindowRounding", style.WindowRounding);
    ini_.SetDoubleValue(section, "FramePaddingX", style.FramePadding.x);
    ini_.SetDoubleValue(section, "FramePaddingY", style.FramePadding.y);
    ini_.SetDoubleValue(section, "FrameRounding", style.FrameRounding);
    ini_.SetDoubleValue(section, "ItemSpacingX", style.ItemSpacing.x);
    ini_.SetDoubleValue(section, "ItemSpacingY", style.ItemSpacing.y);
    ini_.SetDoubleValue(section, "ItemInnerSpacingX", style.ItemInnerSpacing.x);
    ini_.SetDoubleValue(section, "ItemInnerSpacingY", style.ItemInnerSpacing.y);
    ini_.SetDoubleValue(section, "IndentSpacing", style.IndentSpacing);
    ini_.SetDoubleValue(section, "ScrollbarSize", style.ScrollbarSize);
    ini_.SetDoubleValue(section, "ScrollbarRounding", style.ScrollbarRounding);
    ini_.SetDoubleValue(section, "GrabMinSize", style.GrabMinSize);
    ini_.SetDoubleValue(section, "GrabRounding", style.GrabRounding);
    ini_.SetDoubleValue(section, "WindowTitleAlignX", style.WindowTitleAlign.x);
    ini_.SetDoubleValue(section, "WindowTitleAlignY", style.WindowTitleAlign.y);
    ini_.SetDoubleValue(section, "ButtonTextAlignX", style.ButtonTextAlign.x);
    ini_.SetDoubleValue(section, "ButtonTextAlignY", style.ButtonTextAlign.y);

    // 写入所有颜色项（以十六进制字符串形式）
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
    {
        const char *name = ImGui::GetStyleColorName(i);
        const auto color_f4 = style.Colors[i];
        const auto color_u32 = ImGui::ColorConvertFloat4ToU32(color_f4);

        SaveColor(&ini_, section, name, color_u32);
    }

    // 保存到磁盘
    rc = ini_.SaveFile(ini_filepath);
    if (rc < 0)
        return;
}

/**
 * @brief 绘制颜色设置界面
 *
 * @param open 可选项参数控制窗口显示状态
 */
void WindowBase::DrawColorsSettings(bool *open)
{
    ImGui::SetNextWindowPos(s_mainWindowPos);
    ImGui::SetNextWindowSize(s_mainWindowSize);

    ImGui::Begin("Settings", open, s_mainWindowFlags);

    ImGuiStyle &style = ImGui::GetStyle();

    //全局透明度和字体缩放
    ImGui::SliderFloat("Global Alpha", &style.Alpha, 0.2f, 1.0f, "%.2f");
    style.Alpha =
        std::clamp(style.Alpha, 0.2f, 1.f); // 1.f == 1.0f ; 限制透明度范围

    ImGui::SliderFloat("Global Font Scale",
                       &ImGui::GetIO().FontGlobalScale,
                       0.05f,
                       3.00f,
                       "%.1f");

    // 尺寸设置
    ImGui::Text("Sizes");

    ImGui::SliderFloat2("Window Padding",
                        (float *)&style.WindowPadding,
                        0.0f,
                        20.0f,
                        "%.0f");
    ImGui::SliderFloat("Window Rouning",
                       &style.WindowRounding,
                       0.0f,
                       16.0f,
                       "%.0f");

    ImGui::SliderFloat2("Frame Padding",
                        (float *)&style.FrameRounding,
                        0.0f,
                        20.0f,
                        "%.0f");
    ImGui::SliderFloat("Frame Rounding",
                       &style.FrameRounding,
                       0.0f,
                       16.0f,
                       "%.0f");

    ImGui::SliderFloat2("Item Spacing",
                        (float *)&style.ItemSpacing,
                        0.0f,
                        20.0f,
                        "%.0f");
    ImGui::SliderFloat2("Item InnerSpacing",
                        (float *)&style.ItemInnerSpacing,
                        0.0f,
                        20.0f,
                        "%.0f");
    ImGui::SliderFloat("Indent Spacing",
                       &style.ScrollbarSize,
                       1.0f,
                       20.0f,
                       "%.0f");
    ImGui::SliderFloat("Scrollbar Size",
                       &style.ScrollbarSize,
                       1.0f,
                       20.0f,
                       "%.0f");
    ImGui::SliderFloat("Scrollbar Rounding",
                       &style.ScrollbarRounding,
                       0.0f,
                       16.0f,
                       "%.0f");
    ImGui::SliderFloat("Grab MinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
    ImGui::SliderFloat("Grab Rounding",
                       &style.GrabRounding,
                       0.0f,
                       16.0f,
                       "%.0f");

    ImGui::Text("Aligment");
    ImGui::SliderFloat2("Window Title Align",
                        (float *)&style.WindowTitleAlign,
                        0.0f,
                        1.0f,
                        "%.2f");
    ImGui::SliderFloat2("Button Text Align",
                        (float *)&style.ButtonTextAlign,
                        0.0f,
                        1.0f,
                        "%.2f");

    ImGui::Text("Colors");
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
    {
        const char *name = ImGui::GetStyleColorName(i);
        ImGui::PushID(i);
        ImGui::ColorEdit4(name, reinterpret_cast<float *>(&style.Colors[i]));
        ImGui::PopID();
    }

    ImGui::End();
}

/**
 * @brief 设置为默认的Dark主题风格
 *
 * 复制了 ImGui 自带的暗色主题模板,对部分参数做了微调
 *
 * @return ImGuiStyle
 */
ImGuiStyle WindowBase::defaultColorStyle()
{
    ImGuiStyle style = ImGuiStyle();
    ImGui::StyleColorsDark(&style);

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.60f;
    style.WindowPadding = ImVec2(8, 8);
    style.WindowRounding = 0.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(32, 32);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ChildRounding = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(4, 3);
    style.FrameRounding = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.CellPadding = ImVec2(4, 2);
    style.TouchExtraPadding = ImVec2(0, 0);
    style.IndentSpacing = 21.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 14.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 12.0f;
    style.GrabRounding = 0.0f;
    style.LogSliderDeadzone = 4.0f;
    style.TabRounding = 4.0f;
    style.TabBorderSize = 0.0f;
    //style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
    style.SeparatorTextBorderSize = 3.0f;
    style.SeparatorTextAlign = ImVec2(0.0f, 0.5f);
    style.SeparatorTextPadding = ImVec2(20.0f, 3.f);
    style.DisplayWindowPadding = ImVec2(19, 19);
    style.DisplaySafeAreaPadding = ImVec2(3, 3);
    style.MouseCursorScale = 1.0f;
    style.AntiAliasedLines = true;
    style.AntiAliasedLinesUseTex = true;
    style.AntiAliasedFill = true;
    style.CurveTessellationTol = 1.25f;
    style.CircleTessellationMaxError = 0.30f;

    style.Colors[ImGuiCol_Text] = ImVec4(1.0F, 1.0F, 1.0F, 1.0F);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.0F);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(1.0F, 1.0F, 1.0F, 0.0F);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0F, 0.0F, 0.0F, 0.0F);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.0F);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.0F);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0F, 0.0F, 0.0F, 0.51f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.0F);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0F);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] =
        ImVec4(0.41f, 0.41f, 0.41f, 1.0F);
    style.Colors[ImGuiCol_ScrollbarGrabActive] =
        ImVec4(0.51f, 0.51f, 0.51f, 1.0F);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.0F);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.0F);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.0F);
    style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.0F);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.0F);
    style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.0F);
    style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
    style.Colors[ImGuiCol_SeparatorHovered] =
        ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.0F);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    style.Colors[ImGuiCol_ResizeGripHovered] =
        ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive] =
        ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.0F);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0F, 0.43f, 0.35f, 1.0F);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.0F, 1.0F);
    style.Colors[ImGuiCol_PlotHistogramHovered] =
        ImVec4(1.0F, 0.60f, 0.0F, 1.0F);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0F, 1.0F, 0.0F, 0.90f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.0F);
    style.Colors[ImGuiCol_NavWindowingHighlight] =
        ImVec4(1.0F, 1.0F, 1.0F, 0.70f);
    style.Colors[ImGuiCol_NavWindowingDimBg] =
        ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_ModalWindowDimBg] =
        ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    return style;
}
