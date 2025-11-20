#pragma once

#include <array>
#include <chrono>
#include <compare>
#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include <imgui.h>

#include "WindowBase.hpp"

// 日历应用类,继承自WindowBase, 提供日程管理功能
class Calender : public WindowBase
{
public:
    // @brief 会议窗口标志:禁止调整大小,移动,折叠和滚动条
    static constexpr auto s_meetingWindowFlags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

    // 会议窗口尺寸常量
    static constexpr auto s_meetingWindowSize = ImVec2(300.0f, 100.0f);
    // 会议窗口按钮尺寸常量
    static constexpr auto s_meetingWindowButtonSize = ImVec2(120.0f, 0.0);
    // 会议窗口位置常量（屏幕中央）
    static constexpr auto s_meetingWindowPos =
        ImVec2(1280.0f / 2.0f - s_meetingWindowSize.x / 2.0f,
               720.0f / 2.0f - s_meetingWindowSize.y / 2.0f);

    // 月份名称数组
    static constexpr auto s_monthNames = std::array<std::string_view, 12u>{
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December",
    };

    // 支持的最小年份
    static constexpr auto s_minYear = std::int32_t{2000};
    // 支持的最大年份
    static constexpr auto s_maxYear = std::int32_t{2038};

    // 会议结构体:存储会议信息
    struct Meeting
    {
        // 会议名称
        std::string name_;

        // 序列化会议数据到文件
        void Serialize(std::ofstream &out) const;
        // 从文件反序列化到会议数据
        static Meeting Deserialize(std::ifstream &in);

        // 重载相等运算符
        constexpr bool operator==(const Meeting &rhs) const
        {
            return name_ == rhs.name_;
        }
    };

public:
    // 构造函数: 初始化会议映射为空
    Calender() : meetings_({}) {};
    virtual ~Calender() {};

    void Draw(std::string_view label, bool *open = nullptr) final;

    // 从文件中加载会议数据
    void LoadMeetingsFromFile(std::string_view filename);
    // 保存会议文件数据到文件
    void SaveMeetingsToFile(std::string_view filename);

private:
    // 绘制日期选择下拉框(年月日)
    void DrawDateCombo();
    // 绘制日历网格界面
    void DrawCalender();
    // 绘制添加会议窗口
    void DrawAddMeetingWindow();
    // 绘制会议列表
    void DrawMeetingList();

    // 更新选中日期的相关变量
    void UpdateSelectedDateVariables();

private:
    int selectedDay_ = 1;     // 当前选中的日期
    int selectedMonth_ = 1;   // 当前选中的月份
    int selectedYear_ = 2023; // 当前选中的年份
    // 标准日期类型表示选中日期: 年_月_日
    std::chrono::year_month_day selectedDate_{};

    // 添加会议窗口是否打开
    bool addMeetingWindowOpen_ = false;
    // 日历字体大小
    float calenderFontSize_ = 2.0f;

    // 会议数据映射: 按日期数据存储会议列表 <年_月_日, 会议列表>
    std::map<std::chrono::year_month_day, std::vector<Meeting>> meetings_;
};
