#pragma once

#include <GL\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <GLFW\glfw3.h>

/// <summary>
/// 第一人称 Camera(摄像机)类, 实现基于键盘与鼠标的自由视角移动
/// </summary>
class Camera
{
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed);

	void keyControl(bool* keys, GLfloat deltaTime);
	void mouseControl(GLfloat xChange, GLfloat yChange);

	glm::vec3 getCameraPosition();
	glm::vec3 getCameraDirection();

	glm::mat4 calculateViewMatrix();

	~Camera();

private:
	void update();

private:
	glm::vec3 position_{};				///< Camera当前位置
										
	glm::vec3 front_{};					///< Camera当前朝向向量(由 yaw + pitch)计算
	glm::vec3 up_{};					///< Camera当前 上方(Up)向量
	glm::vec3 right_{};					///< Camera当前右向量, right = normalize(cross(front, worldUp))
	glm::vec3 worldUp_{};				///< 世界坐标系 上方向量
										
	GLfloat yaw_{};						///< 偏航角 (水平旋转)
	GLfloat pitch_{};					///< 俯仰角 (垂直旋转)
										
	GLfloat moveSpeed_{};				///< 键盘移动速度
	GLfloat turnSpeed_{};				///< 鼠标灵敏度
};

