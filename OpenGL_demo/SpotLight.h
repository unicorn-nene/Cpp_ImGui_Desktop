#pragma once
#include "PointLight.h"

/// <summary>
/// SpotLight(聚光灯)类, 继承自 PointLight,
/// 用于模拟聚光灯效果: 从一个点位置向指定方向发射光线, 并且只有在锥形区域内的片元才会被照亮.
/// </summary>
class SpotLight :
	public PointLight
{
public:
	SpotLight();

	SpotLight(GLuint shadowWidth, GLuint shadowHeight,
			  GLfloat near, GLfloat far, 
			  GLfloat red, GLfloat green, GLfloat blue,
			  GLfloat aIntensity, GLfloat dIntensity,
			  GLfloat xPos, GLfloat yPos, GLfloat zPos,
			  GLfloat xDir, GLfloat yDir, GLfloat zDir,
			  GLfloat con, GLfloat lin, GLfloat exp,
			  GLfloat edg);

	void UseLight(GLuint ambientIntensityLocation, GLuint ambientColourLocation,
		          GLuint diffuseIntensityLocation, GLuint positionLocation, GLuint directionLocation,
		          GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation,
		          GLuint edgeLocation);

	void SetFlash(glm::vec3 pos, glm::vec3 dir);

	void Toggle() { isOn_ = !isOn_; }

	~SpotLight();

private:
	glm::vec3 direction_{};				 ///< 聚光灯方向向量(决定光锥的指向)
	GLfloat edge_{};                     ///< 光照锥边缘单位(单位:度), 表示光照的有效区域, 超出该角度的区域会迅速衰减.
	GLfloat procEdge_{};				 ///< 预计算后的光照边缘余弦值(用于在着色器中快速比较光线角度)

	bool isOn_{};						 ///< 聚光灯是否开启（true 表示发光，false 表示关闭）
};

