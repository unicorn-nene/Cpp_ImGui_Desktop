#include <cstdint>
#include <filesystem>
#include <iostream>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl3_loader.h"
#include <imgui.h>
#include <implot.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

#include <GLFW/glfw3.h>

#include "Desktop.hpp"

#if defined(_MSC_VER) && (_MSC_VER >= 1900) &&                                 \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// 全局常量定义
constexpr auto WINDOW_WIDTH = std::uint32_t{1280};
constexpr auto WINDOW_HEIGHT = std::uint32_t{720};

// c++ 17 文件系统命名空间
namespace fs = std::filesystem;

// 函数声明:GLFW 错误回调函数
static void glfw_error_callback(int error, const char *descroption);

/**
 * @brief 开始一帧渲染循环的准备阶段
 *
 */
void start_cycle()
{
    glfwPollEvents(); // 处理所有挂起的事件

    ImGui_ImplOpenGL3_NewFrame(); // 通知 ImGui/OpenGL3 后端开始新的一帧
    ImGui_ImplGlfw_NewFrame();    // 通知 ImGui/GLFW 后端开始新的一帧
}

/**
 * @brief 完成一帧渲染并交换缓冲区
 *
 * @param window 当前的 GLFW 窗口对象
 */
void end_cycle(GLFWwindow *const window)
{
    // 定义背景清屏颜色
    const auto clear_color = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

    // 获取当前窗口的帧缓冲尺寸(像素)
    int display_w = 0;
    int display_h = 0;
    glfwGetFramebufferSize(window, &display_w, &display_h);

    // 设置 OpenGL 视口
    glViewport(0, 0, display_w, display_h);

    // 清空颜色缓冲, w: Alpha 分量
    glClearColor(clear_color.x * clear_color.w,
                 clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w,
                 clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    // 渲染 ImGui 绘制数据到屏幕
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 交换前后缓冲区(显示新的一帧)
    glfwSwapBuffers(window);
}


int main(int, char **)
{
    // 初始化 GLFW 并设置错误回调
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        return 1;
    }

// OpenGL 版本
#if defined(IMGUI_IMPL_OPENGL_32)
    // OpenGL ES 2.0 环境(移动平台)
    const char *glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIEMT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // masOS: OpenGL 3.2 + GLSL 150 核心模式
    const char *glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // mac 必须
#else
    // 默认: OpenGL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // 创建窗口
    auto *window = glfwCreateWindow(static_cast<std::int32_t>(WINDOW_WIDTH),
                                    static_cast<std::int32_t>(WINDOW_HEIGHT),
                                    "Gui",
                                    nullptr,
                                    nullptr);
    if (!window)
        return 1;

    // 激活 OpenGL 上下文并开启垂直同步(vsync)
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // 初始化 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::CreateContext(); // 创建 ImPlot 上下文(用于绘图)

    // 初始化 ImGui 平台层与渲染层(GLFW + OpenGL3)
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // 创建主界面对象
    Desktop window_obj;

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        start_cycle();

        ImGui::NewFrame();
        render(window_obj);
        ImGui::Render();

        end_cycle(window);
    }

    // 程序结束前保存主题设置
    window_obj.SaveTheme();

    // 清理资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

/**
 * @brief GLFW 错误回调函数
 *
 */
void glfw_errro_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW Error %d : %s\n", error, description);
}
