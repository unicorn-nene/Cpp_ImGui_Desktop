#pragma once

#include <GL\glew.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "ShadowMap.h"

/// <summary>
/// 光源基类, 包含光源颜色,环境光强度,漫反射强度基本信息
/// </summary>
class Light
{
public:
	Light();
	Light(GLuint shadowWidth, GLuint shadowHeight, 
			GLfloat red, GLfloat green, GLfloat blue,
			GLfloat aIntensity, GLfloat dIntensity);

	ShadowMap* getShadowMap() { return shadowMap_; }

	~Light();

protected:
	glm::vec3 colour_{};					 ///< 光源颜色(RGB)
	GLfloat ambientIntensity_{};			 ///< 环境光强度
	GLfloat diffuseIntensity_{};			 ///< 漫反射强度

	glm::mat4 lightProj_{};					 ///< 光源投影矩阵, 用于阴影计算

	ShadowMap* shadowMap_{};				 ///< 阴影贴图对象指针(深度纹理)
};

