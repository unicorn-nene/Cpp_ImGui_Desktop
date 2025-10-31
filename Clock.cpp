#include <chrono>
#include <cmath>
#include <string_view>
#include <tuple>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "Clock.hpp"

void Clock::Draw(std::string_view label, bool *open)
{
    ImGui::SetNextWindowSize(s_mainWindowSize);
    ImGui::SetNextWindowPos(s_mainWindowPos);

    ImGui::Begin(label.data(), nullptr, s_mainWindowFlags);

    const auto cursor_pos = ImGui::GetCursorScreenPos();

    center_ =
        ImVec2(cursor_pos.x + s_circleRadius, cursor_pos.y + s_circleRadius);

    DrawCircle(s_circleRadius);

    GetTime();
    const auto [hour_theta, minute_theta, second_thate] = GetTheta();

    DrawClockHand(s_circleRadius * s_hrsClockHandLength,
                  hour_theta,
                  ImColor(1.0f, 0.0f, 0.0f, 1.0f));
    DrawClockHand(s_circleRadius * s_minsClockHandLength,
                  minute_theta,
                  ImColor(0.0f, 1.0f, 0.0f, 1.0f));
    DrawClockHand(s_circleRadius * s_secsClockHandLength,
                  second_thate,
                  ImColor(0.0f, 0.0f, 1.0f, 1.0f));

    DrawAllHourStrokes();
    DrawAllMinutesStrokes();

    ImGui::End();
}


/**
* @brief 绘制表盘圆环
*
* @param radius 表盘半径
*/
void Clock::DrawCircle(const float radius)
{
    ImGui::GetWindowDrawList()->AddCircle(center_,
                                          radius,
                                          ImGui::GetColorU32(ImGuiCol_Text),
                                          100,
                                          2.0f);
}

/**
 * @brief 绘制单个时钟指针
 *
 * @param radius 指针长度比例
 * @param theta 当前指针角度
 * @param color 指针颜色
 *
 * 通过三角函数计算出指针的终点坐标，然后绘制中心点到终点的直线。
 * 注意此处坐标系相对屏幕左上角为原点，Y 轴向下，因此计算时对符号取反。
 */
void Clock::DrawClockHand(const float radius,
                          const float theta,
                          const ImColor color)
{
    const auto c = std::cos(theta); //x
    const auto s = std::sin(theta); //y

    // 计算指针终点坐标
    const auto end_point =
        ImVec2(center_.x - radius * c, center_.y - radius * s);

    // 从中心绘制到终点的一条线
    ImGui::GetWindowDrawList()->AddLine(center_, end_point, color, 3.0f);
}

/**
 * @brief 绘制表盘的12个小时
 *
 * 每隔 30° 绘制一条短线，长度较粗，
 * 代表时钟上的 1~12 小时位置。
 */
void Clock::DrawAllHourStrokes()
{
    for (std::int32_t hr = 0; hr < 12; ++hr)
    {
        // 当前小时对应的角度(0 ~ 2Π),加上偏移量使得12点向上
        const auto theta =
            (static_cast<float>(hr) * ((2.0f * s_PI) / 60.0f)) + s_offset;
        const auto c = std::cos(theta);
        const auto s = std::sin(theta);

        // 刻度起点 -> 接近外圈
        const auto start_point =
            ImVec2(center_.x + (s_circleRadius * s_hrsStrokesLength) * c,
                   center_.y + (s_circleRadius * s_hrsStrokesLength) * c);

        // 刻度终点 -> 表盘边缘
        const auto end_point = ImVec2(center_.x + s_circleRadius * c,
                                      center_.y + s_circleRadius * s);

        // 绘制刻度线(较粗)
        ImGui::GetWindowDrawList()->AddLine(start_point,
                                            end_point,
                                            ImGui::GetColorU32(ImGuiCol_Text),
                                            2.0f);
    }
}


/**
 * @brief 绘制表盘的60个分钟刻度线
 *
 * 每隔 6° 绘制一条细刻度，用于表示分钟。
 * 每5条分钟刻度与小时刻度重叠。
 */
void Clock::DrawAllMinutesStrokes()
{
    for (std::uint32_t min = 0; min < 60; ++min)
    {
        const auto theta =
            (static_cast<float>(min) * ((2.0f * s_PI) / 60.0f)) + s_offset;
        const auto c = std::cos(theta);
        const auto s = std::sin(theta);

        const auto start_point =
            ImVec2(center_.x + (s_circleRadius * s_minsClockHandLength) * c,
                   center_.y + (s_circleRadius * s_minsClockHandLength) * c);

        const auto end_point = ImVec2(center_.x + s_circleRadius * c,
                                      center_.y + s_circleRadius * s);

        // 绘制细刻度线
        ImGui::GetWindowDrawList()->AddLine(start_point,
                                            end_point,
                                            ImGui::GetColorU32(ImGuiCol_Text),
                                            1.0f);
    }
}

/**
 * @brief 绘制数字时钟文本(如 14:30:22)
 *
 * 直接使用 ImGui::Text() 输出当前小时,分钟,秒
 *
 */
void Clock::DrawDigitalClock()
{
    ImGui::Text("%d:%d:%d", hrs_, mins_, secs_);
}

/**
 * @brief 从系统时间中获取当前时间
 *
 * 使用 std::chrono 获取系统时间，
 * 并通过 std::localtime() 转换成本地时间结构。
 */
void Clock::GetTime()
{
    const auto timestamp_nopw = std::chrono::system_clock::now(); // 当前时间点
    const auto time_now = std::chrono::system_clock::to_time_t(timestamp_nopw);
    const auto time_struct = std::localtime(&time_now); // 转换为 tm 结构

    // 更新成员变量
    secs_ = time_struct->tm_sec;
    mins_ = time_struct->tm_min;
    hrs_ = time_struct->tm_hour;
}

/**
 * @brief 计算 时针,分针,秒针对应的角度
 *
 * @return std::tuple<float, float, float> 包含三个角度的元组
 *
 *  每个指针的角度基于当前时间计算，使用弧度制表示：
 *
 * - 秒针：每60秒转一圈。
 * - 分针：每60分钟转一圈，并随秒针平滑移动。
 * - 时针：每12小时转一圈，并随分钟、秒针微调。
 */
std::tuple<float, float, float> Clock::GetTheta()
{
    // 当前时间分数形式(用于连续平滑移动)
    const auto seconds_frac = static_cast<float>(secs_);
    const auto minutes_frac = static_cast<float>(mins_) + seconds_frac / 60.0f;
    const auto hours_frac = static_cast<float>(hrs_) + minutes_frac / 60.0f;

    // 将时间转换为 角度(弧度)

    const auto hour_theta = (hours_frac * ((2.0f * s_PI) / 12.0f)) + s_offset;
    const auto minute_theta =
        (minutes_frac * ((2.0f * s_PI) / 60.0f)) + s_offset;

    const auto second_theta =
        (seconds_frac * ((2.0f * s_PI) / 60.0f)) + s_offset;

    return std::make_tuple(hour_theta, minute_theta, second_theta);
}
