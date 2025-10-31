#pragma once

#include <cstdint>
#include <numbers>
#include <string_view>
#include <tuple>

#include <imgui.h>

#include "WindowBase.hpp"

/**
 * @class Clock
 * @brief 以 ImGui 绘制的模拟与数字时钟窗口。
 *
 */

class Clock : public WindowBase
{
public:
    static constexpr auto s_PI = std::numbers::pi_v<float>; // 圆周率常量
    static constexpr auto s_circleRadius = 250.0f;          // 表盘半径
    static constexpr auto s_offset = s_PI / 250.0f;         // 坐标系偏移量
    static constexpr auto s_innerRadius = 5.0f;             // 表盘中心点半径
    static constexpr auto s_hrsClockHandLength = 0.95f;     // 时针长度系数
    static constexpr auto s_minsClockHandLength = 0.85f;    // 分针长度系数
    static constexpr auto s_secsClockHandLength = 0.75f;    // 秒针长度系数
    static constexpr auto s_hrsStrokesLength = 0.90f;       // 小时刻度系数
    static constexpr auto s_minsStrokesLength = 0.95f;      // 分钟刻度系数

public:
    Clock() : secs_(0), mins_(0), hrs_(0), center_({}) {};
    virtual ~Clock();

    void Draw(std::string_view label, bool *open = nullptr) final;

    void GetTime();

private:
    void DrawCircle(const float radius);
    /**
     * @brief 绘制单个指针
     *
     * @param radius
     * @param theta
     * @param color
     */
    void DrawClockHand(const float radius,
                       const float theta,
                       const ImColor color);
    void DrawAllHourStrokes();
    void DrawAllMinutesStrokes();
    void DrawDigitalClock();

    std::tuple<float, float, float> GetTheta();

public:
    std::int32_t secs_{};
    std::int32_t mins_{};
    std::int32_t hrs_{};

private:
    ImVec2 center_{};
};
