#pragma once

#include <string_view>
#include <vector>

#include <imgui.h>

#include <fmt/format.h>
#include <implot.h>

#include "AdvCalc.hpp"
#include "Calender.hpp"
#include "Clock.hpp"
#include "CsvEditor.hpp"
#include "Diff.hpp"
#include "FileExplorer.hpp"
#include "OtherTopics.hpp"
#include "Paint.hpp"
#include "TextEditor.hpp"
#include "WindowBase.hpp"

/**
 * @brief 模拟桌面系统的主窗口类.负责管理多个子窗口(应用)和桌面图标的控制
 *
 */
class Desktop : public WindowBase
{
    struct Icon
    {
        Icon(std::string_view label, WindowBase *base)
            : label_(label), base_(base_), position_(ImVec2({})) {};
        void Draw();

        std::string label_;
        ImVec2 position_{};
        bool popupOpen_ = false;
        std::uint32_t clickCount_ = 0;
        WindowBase *base_ = nullptr;
    };

public:
    // 初始化所有内置应用
    Desktop()
        : icons({}), adv_calc_({}), calender_({}), diff_viewer_({}),
          file_explorer_({}), paint_({}), text_editor_({}), csv_editor_({}),
          clock_({})
    {
        icons.reserve(7);
        icons.push_back(Icon{"AdvCalc", &adv_calc_});
        icons.push_back(Icon{"Calender", &calender_});
        icons.push_back(Icon{"DiffViewer", &diff_viewer_});
        icons.push_back(Icon{"FileExplorer", &file_explorer_});
        icons.push_back(Icon{"Paint", &paint_});
        icons.push_back(Icon{"TextEditor", &text_editor_});
        icons.push_back(Icon{"CsvEditor", &csv_editor_});
        icons.push_back(Icon{"OtherTopics", &other_topics_});
    }

    // 绘制桌面主窗口内容
    void Draw(std ::string_view label, bool *open = nullptr) final;

    void DrawBackground(); // 绘制桌面背景
    void DrawDesktop();    // 绘制桌面图标
    void DrawTaskbar();    // 绘制任务栏,包括时间,应用切换信息

    // 显示图标列表窗口
    void ShowIconList(bool *open = nullptr);

private:
    std::vector<Icon> icons{}; // 桌面图标的集合

    /* 各个子窗口(应用)的实例 */

    AdvCalc adv_calc_{};
    Calender calender_{};
    DiffViewer diff_viewer_{};
    FileExplorer file_explorer_{};
    Paint paint_{};
    TextEditor text_editor_{};
    CsvEditor csv_editor_{};
    OtherTopics other_topics_{};

    Clock clock_{}; // 时针部件,用于任务栏显示时间
};

void render(Desktop &window_class);
