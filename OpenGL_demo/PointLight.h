#pragma once
#include "Light.h"

#include <vector>

#include "OmniShadowMap.h"

/// <summary>
/// 点光源是从一个点向所有方向均匀发射光线的光源，具有位置属性和衰减特性
/// </summary>
class PointLight :
	public Light
{
public:
	PointLight();
	PointLight(GLuint shadowWidth, GLuint shadowHeight,
			   GLfloat near, GLfloat far,
			   GLfloat red, GLfloat green, GLfloat blue,
			   GLfloat aIntensity, GLfloat dIntensity,
			   GLfloat xPos, GLfloat yPos, GLfloat zPos,
			   GLfloat con, GLfloat lin, GLfloat exp);

	void UseLight(GLuint ambientIntensityLocation, GLuint ambientColourLocation,
		          GLuint diffuseIntensityLocation, GLuint positionLocation,
		          GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation);

	std::vector<glm::mat4> CalculateLightTransform();
	GLfloat GetFarPlane();
	glm::vec3 GetPosition();

	~PointLight();

protected:
	glm::vec3 position_{};   ///< 点光源位置, 世界坐标系坐标
	GLfloat constant_{};     ///< 衰减常量
	GLfloat linear_{};       ///< 衰减一次项
	GLfloat exponent_{};     ///< 衰减二次项

	GLfloat farPlane_{};     ///< 阴影映射的远裁剪平面距离
};

