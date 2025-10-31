#pragma once
#include "WindowBase.hpp"
#include <string_view>

/**
 * @brief 展示多种ImGui 控件示例的窗口类
 *
 * 该窗口主要用于演示不同类型的 ImGui 控件布局，
 * 包括 TreeNode、CollapsingHeader、TabBar 等。
 * 可作为 UI 组件测试或学习的参考窗口。
 */
class OtherTopics : public WindowBase
{
public:
    OtherTopics();
    ~OtherTopics();

    void Draw(std::string_view label, bool *open = nullptr) final;
};
