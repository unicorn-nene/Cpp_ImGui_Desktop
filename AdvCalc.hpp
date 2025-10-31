#pragma once

#include <array>
#include <set>
#include <string_view>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "WindowBase.hpp"

// 高级计算器类,继承自 类WindowBase, 提供函数绘图功能
class AdvCalc : public WindowBase
{
public:
    // 函数名称数组常量: 包含可用函数的显示名称
    constexpr static auto s_functionNames =
        std::array<std::string_view, 3>{"unk", "sin(x)", "cos(x)"};

    // 函数枚举类型:定义可用的数学函数
    enum class Function
    {
        NONE,
        SIN,
        COS,
    };

public:
    AdvCalc() : m_selectedFunctions({}) {};
    virtual ~AdvCalc();

    void Draw(std::string_view label, bool *open = nullptr) final;

private:
    // 绘制函数选择界面
    void DrawSelection();
    // 绘制函数图像
    void DrawPlot();

    // 函数名称映射:将字符串映射到 Function 枚举
    Function functionNameMapping(std::string_view function_name);
    // 函数求值:根据函数类型和 x值计算结果
    double evaluateFunction(const Function function, const double x);

public:
    // 当前选中的函数集合(使用set避免重复)
    std::set<Function> m_selectedFunctions{};
};
