#pragma once
#include "Light.h"

/// <summary>
/// 定向光/方向光/平行光 光源类：用于模拟太阳等无限远光源。
/// 光线方向一致，不随相机位置改变。
/// </summary>
class DirectionalLight :
	public Light
{
public:
	DirectionalLight();
	DirectionalLight(GLuint shadowWidth, GLuint shadowHeight, 
					GLfloat red, GLfloat green, GLfloat blue,
					GLfloat aIntensity, GLfloat dIntensity,
					GLfloat xDir, GLfloat yDir, GLfloat zDir);

	void UseLight(GLint ambientIntensityLocation, GLint ambientColourLocation, GLint diffuseIntensityLocation, GLint directionLocation);

	glm::mat4 CalculateLightTransform();

	~DirectionalLight();

private:
	glm::vec3 direction_{};				///< 光源方向 (从物体指向光源的方向取反)
};

