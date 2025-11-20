#include "DirectionalLight.h"

/// <summary>
/// 默认构造白色方向光，方向为 (0, -1, 0), 即向下照射
/// </summary>
DirectionalLight::DirectionalLight()
	:Light()
	,direction_(glm::vec3(0.0f, -1.0f, 0.0f))
{
	
}

/// <summary>
/// 创建方向光源并初始化阴影贴图
/// </summary>
/// <param name="shadowWidth">阴影贴图宽度</param>
/// <param name="shadowHeight">阴影贴图高度</param>
/// <param name="red">光源颜色 R 分量</param>
/// <param name="green">光源颜色 G 分量</param>
/// <param name="blue">光源颜色 B 分量</param>
/// <param name="aIntensity">环境光强度</param>
/// <param name="dIntensity">漫反射光强度</param>
/// <param name="xDir">光方向 X 分量</param>
/// <param name="yDir">光方向 Y 分量</param>
/// <param name="zDir">光方向 Z 分量</param>
DirectionalLight::DirectionalLight(GLuint shadowWidth, GLuint shadowHeight, 
									GLfloat red, GLfloat green, GLfloat blue,
									GLfloat aIntensity, GLfloat dIntensity,
									GLfloat xDir, GLfloat yDir, GLfloat zDir) 
	: Light(shadowWidth, shadowHeight, red, green, blue, aIntensity, dIntensity)
	, direction_(glm::vec3(xDir, yDir, zDir))
{
	// 定向光使用正交投影
	lightProj_ = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.0f);
}

/// <summary>
/// 向 Shader 上传光源数据的 uniform 变量
/// </summary>
/// <param name="ambientIntensityLocation">环境光强度 uniform</param>
/// <param name="ambientColourLocation">光颜色 uniform</param>
/// <param name="diffuseIntensityLocation">漫反射光强度 uniform</param>
/// <param name="directionLocation">光方向 uniform</param>
void DirectionalLight::UseLight(GLint ambientIntensityLocation, GLint ambientColourLocation, GLint diffuseIntensityLocation, GLint directionLocation)
{
	glUniform3f(ambientColourLocation, colour_.x, colour_.y, colour_.z);
	glUniform1f(ambientIntensityLocation, ambientIntensity_);

	glUniform3f(directionLocation, direction_.x, direction_.y, direction_.z);
	glUniform1f(diffuseIntensityLocation, diffuseIntensity_);
}

/// <summary>
/// 获取方向光用于阴影渲染的光源变换矩阵, 该矩阵将世界坐标转换到光源视角（Light Space）.
/// 光源变换矩阵 = 光源投影矩阵 * 光源视图矩阵
/// </summary>
/// <returns>Light Space Matrix（光源空间变换矩阵）</returns>
glm::mat4 DirectionalLight::CalculateLightTransform()
{
	return lightProj_ * glm::lookAt(-direction_, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

/// <summary>
/// 析构函数（父类释放 shadowMap_）
/// </summary>
DirectionalLight::~DirectionalLight()
{
}
