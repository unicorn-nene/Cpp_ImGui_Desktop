#include "Camera.h"

/// <summary>
/// 默认构造函数, 使用原地位置,默认方向初始化
/// </summary>
Camera::Camera()
{}

/// <summary>
/// 指定初始位置、世界 Up、Yaw、Pitch、移动速度与旋转速度的构造函数。
/// </summary>
/// <param name="startPosition">摄像机初始位置</param>
/// <param name="startUp">世界坐标系的 Up 向量</param>
/// <param name="startYaw">初始偏航角（绕 Y 轴）</param>
/// <param name="startPitch">初始俯仰角（绕 X 轴）</param>
/// <param name="startMoveSpeed">键盘移动速度</param>
/// <param name="startTurnSpeed">鼠标旋转灵敏度</param>
Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed)
	:position_(startPosition)
	, front_(glm::vec3(0.0f, 0.0f, -1.0f))
	,worldUp_(startUp)
	,yaw_(startYaw)
	,pitch_(startPitch)
	,moveSpeed_(startMoveSpeed)
	,turnSpeed_(startTurnSpeed)
{
	update();
}

/// <summary>
/// 根据键盘输入(前后左右)更新摄像机位置
/// </summary>
/// <param name="keys">按键状态数组</param>
/// <param name="deltaTime">每帧之间的间隔时间</param>
void Camera::keyControl(bool* keys, GLfloat deltaTime)
{
	GLfloat velocity = moveSpeed_ * deltaTime;

	if (keys[GLFW_KEY_W])
	{
		position_ += front_ * velocity;
	}

	if (keys[GLFW_KEY_S])
	{
		position_ -= front_ * velocity;
	}

	if (keys[GLFW_KEY_A])
	{
		position_ -= right_ * velocity;
	}

	if (keys[GLFW_KEY_D])
	{
		position_ += right_ * velocity;
	}
}

/// <summary>
/// 根据鼠标移动量更新 Camera 的 yaw 和 pitch, 从而改变 Camera 方向
/// </summary>
/// <param name="xChange">鼠标 X 方向变化量</param>
/// <param name="yChange">鼠标 Y 方向变化量</param>
void Camera::mouseControl(GLfloat xChange, GLfloat yChange)
{
	xChange *= turnSpeed_;
	yChange *= turnSpeed_;

	yaw_ += xChange;
	pitch_ += yChange;

	if (pitch_ > 89.0f)
	{
		pitch_ = 89.0f;
	}

	if (pitch_ < -89.0f)
	{
		pitch_ = -89.0f;
	}

	update();
}

/// <summary>
/// 获取用于渲染的 view (视图)矩阵
/// </summary>
/// <returns>view (视图)矩阵</returns>
glm::mat4 Camera::calculateViewMatrix()
{
	return glm::lookAt(position_, position_ + front_, up_);
}

/// <summary>
/// 获取 Camera 当前位置
/// </summary>
/// <returns>Camera在世界空间中的位置向量</returns>
glm::vec3 Camera::getCameraPosition()
{
	return position_;
}

/// <summary>
/// 获取 Camera 的观察方向
/// </summary>
/// <returns>Camera朝向的单位方向向量</returns>
glm::vec3 Camera::getCameraDirection()
{
	return glm::normalize(front_);
}

/// <summary>
/// 根据 yaw 和 pitch 更新 front/right/up 向量
/// </summary>
void Camera::update()
{
	// 使用欧拉角计算 Camera 的前方向
	front_.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
	front_.y = sin(glm::radians(pitch_));
	front_.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
	front_ = glm::normalize(front_);

	// 右方向 = 前方向 × 世界上方向
	right_ = glm::normalize(glm::cross(front_, worldUp_));
	// 上方向 = 右方向 × 前方向
	up_ = glm::normalize(glm::cross(right_, front_));
}

/// <summary>
/// 析构函数
/// </summary
Camera::~Camera()
{
}
