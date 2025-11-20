#pragma once

#include "stdio.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

/// <summary>
/// Window 类 封装 GLFW 窗口与输入事件
/// 该 Window 类负责:
/// - 创建并初始化 OpenGL 窗口
/// - 管理帧缓冲区大小
/// - 记录键盘输入状态
/// - 记录鼠标移动输入
/// - 提供窗口关闭判断与缓冲区交换控制
/// </summary>
class Window
{
public:
	Window();

	Window(GLint windowWidth, GLint windowHeight);

	int Initialise();

	GLint getBufferWidth() { return bufferWidth_; }
	GLint getBufferHeight() { return bufferHeight_; }

	bool getShouldClose() { return glfwWindowShouldClose(mainWindow_); }

	bool* getsKeys() { return keys_; }
	GLfloat getXChange();
	GLfloat getYChange();

	void swapBuffers() { glfwSwapBuffers(mainWindow_); }

	~Window();

private:
	void createCallbacks();
	static void handleKeys(GLFWwindow* window, int key, int code, int action, int mode);
	static void handleMouse(GLFWwindow* window, double xPos, double yPos);

private:
	GLFWwindow* mainWindow_;			///< 主窗口指针

	GLint width_{};						///< 窗口宽度
	GLuint height_{};					///< 窗口高度
	GLint bufferWidth_{};				///< 实际帧缓冲区宽度
	GLint bufferHeight_{};				///< 实际帧缓冲区高度

	bool keys_[1024]{ false };			///< 键盘按键状态表

	GLfloat lastX_{};					///< 上一次鼠标 X 坐标
	GLfloat lastY_{};					///< 上一次鼠标 Y 坐标
	GLfloat xChange_{};					///< 鼠标 X 偏移量
	GLfloat yChange_{};					///< 鼠标 Y 偏移量
	bool mouseFirstMoved_{};			///< 判断是否第一个采集鼠标坐标
};