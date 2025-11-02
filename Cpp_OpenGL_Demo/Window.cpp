#include "Window.h"


/// <summary>
/// 默认构造函数, 窗口大小设置为 800 * 600
/// </summary>
Window::Window()
	:width_(800)
	, height_(600)
	, lastX_(0.0f)
	, lastY_(0.0f)
	, xChange_(0.0f)
	, yChange_(0.0f)
	, mouseFirstMoved_(true)
{
	
}

/// <summary>
/// 构造函数,自定义窗口宽度与高度
/// </summary>
/// <param name="windowWidth">窗口宽度</param>
/// <param name="windowHeight">窗口高度</param>
Window::Window(GLint windowWidth, GLint windowHeight)
	:width_(windowWidth)
	, height_(windowHeight)
	, lastX_(0.0f)
	, lastY_(0.0f)
	, xChange_(0.0f)
	, yChange_(0.0f)
	, mouseFirstMoved_(true)
{
	
}

/// <summary>
///  初始化 GLFW, GLEW, 创建窗口并注册回调函数
/// </summary>
/// <returns>返回 0 表示成功, 返回 1 表示失败</returns>
int Window::Initialise()
{
	if (!glfwInit())
	{
		printf("Error Initialising GLFW");
		glfwTerminate();
		return 1;
	}

	// Setup GLFW Windows Properties
	// OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Core Profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Allow forward compatiblity
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create the window
	mainWindow_ = glfwCreateWindow(width_, height_, "Test Window", NULL, NULL);
	if (!mainWindow_)
	{
		printf("Error creating GLFW window!");
		glfwTerminate();
		return 1;
	}

	// Get buffer size information
	glfwGetFramebufferSize(mainWindow_, &bufferWidth_, &bufferHeight_);

	// Set the current context
	glfwMakeContextCurrent(mainWindow_);

	// Handle Key + Mouse Input
	createCallbacks();
	glfwSetInputMode(mainWindow_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Allow modern extension access
	glewExperimental = GL_TRUE;

	// init GLEW
	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		printf("Error: %s", glewGetErrorString(error));
		glfwDestroyWindow(mainWindow_);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);

	// Create Viewport
	glViewport(0, 0, bufferWidth_, bufferHeight_);

	glfwSetWindowUserPointer(mainWindow_, this);

	return 0;
}

/// <summary>
/// 鼠标(每帧)移动的水平增量
/// </summary>
/// <returns></returns>
GLfloat Window::getXChange()
{
	GLfloat theChange = xChange_;
	xChange_ = 0.0f;
	return theChange;
}

/// <summary>
/// 鼠标(每帧)移动的垂直增量
/// </summary>
/// <returns></returns>
GLfloat Window::getYChange()
{
	GLfloat theChange = yChange_;
	yChange_ = 0.0f;
	return theChange;
}

/// <summary>
/// 析构函数
/// </summary>
Window::~Window()
{
	glfwDestroyWindow(mainWindow_);
	glfwTerminate();
}


/// =================== private function ===========================

/// <summary>
/// 注册输入事件回调函数, 包括键盘和鼠标移动事件.
/// 该函数会调用 GLEW 的回调机制, 将按键与鼠标事件关联到本类的静态函数成员
/// </summary>
void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow_, handleKeys);
	glfwSetCursorPosCallback(mainWindow_, handleMouse);
}

/// <summary>
/// 自定义 GLFW 键盘事件回调函数
/// 处理键盘按键按下与释放事件,更新 Window类中的按键状态数组
/// </summary>
/// <param name="window">触发事件的 GLFW 窗口指针</param>
/// <param name="key">触发事件的按键值, 对应 GLFW_KEY_*</param>
/// <param name="code">扫描码(一般不使用)</param>
/// <param name="action">按键动作: GLFW_PRESS或者 GLFW_RELEASE</param>
/// <param name="mode">修饰按键状态,如: shift或 control</param>
void Window::handleKeys(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			theWindow->keys_[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			theWindow->keys_[key] = false;
		}
	}
}

/// <summary>
/// GLFW 鼠标移动事件回调函数.
/// 计算当前鼠标位置与上次位置差值, 主要用于相机视角旋转控制
/// </summary>
/// <param name="window">触发事件的 GLFW窗口指针</param>
/// <param name="xPos">鼠标当前的 X 坐标(相对 OpenGL窗口)</param>
/// <param name="yPos">鼠标当前的 Y 坐标(相对 OpenGL窗口)</param>
void Window::handleMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved_)
	{
		theWindow->lastX_ = static_cast<GLfloat>(xPos);
		theWindow->lastY_ = static_cast<GLfloat>(yPos);
		theWindow->mouseFirstMoved_ = false;
		return; // 第一次只记录，不计算差值
	}

	// 先计算差值
	theWindow->xChange_ = static_cast<GLfloat>(xPos - theWindow->lastX_);
	theWindow->yChange_ = static_cast<GLfloat>(theWindow->lastY_ - yPos); // 反转 Y 视角控制

	// 再更新上一次坐标
	theWindow->lastX_ = static_cast<GLfloat>(xPos);
	theWindow->lastY_ = static_cast<GLfloat>(yPos);

	//if (theWindow->mouseFirstMoved_)
	//{
	//	theWindow->lastX_ = xPos;
	//	theWindow->lastY_ = yPos;
	//	theWindow->mouseFirstMoved_ = false;
	//	return; //第一次只初始化，不计算差值
	//}

	//theWindow->xChange_ = xPos - theWindow->lastX_;
	//theWindow->yChange_ = theWindow->lastY_ - yPos; // 反转 Y 视角控制

	//theWindow->lastX_ = xPos;
	//theWindow->lastY_ = yPos;
}


