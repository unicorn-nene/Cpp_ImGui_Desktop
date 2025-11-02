#include "PointLight.h"

/// <summary>
/// 默认构造函数, 初始化一个位于原点, 无衰减的白色点光源
/// </summary>
PointLight::PointLight() 
	: Light()
	, position_(glm::vec3(0.0f, 0.0f, 0.0f))
	, constant_(1.0f)	
	, linear_(0.0f)
	, exponent_(0.0f)
{
}

/// <summary>
/// 构造一个带阴影的点光源。
/// 点光源使用立方体贴图（Cubemap）渲染阴影，需要 6 个视角方向。
/// </summary>
/// <param name="shadowWidth">阴影贴图宽度</param>
/// <param name="shadowHeight">阴影贴图高度</param>
/// <param name="near">光源近裁面</param>
/// <param name="far">光源远裁面（决定阴影采样深度范围）</param>
/// <param name="red">光颜色 R 分量</param>
/// <param name="green">光颜色 G 分量</param>
/// <param name="blue">光颜色 B 分量</param>
/// <param name="aIntensity">环境光强度</param>
/// <param name="dIntensity">漫反射光强度</param>
/// <param name="xPos">光源位置 X 分量</param>
/// <param name="yPos">光源位置 Y 分量</param>
/// <param name="zPos">光源位置 Z 分量</param>
/// <param name="con">衰减常数项</param>
/// <param name="lin">衰减线性项</param>
/// <param name="exp">衰减指数项（二次项）</param>
PointLight::PointLight(GLuint shadowWidth, GLuint shadowHeight,
					   GLfloat near, GLfloat far, 
					   GLfloat red, GLfloat green, GLfloat blue,
					   GLfloat aIntensity, GLfloat dIntensity, 
					   GLfloat xPos, GLfloat yPos, GLfloat zPos, 
					   GLfloat con, GLfloat lin, GLfloat exp) 
	: Light(shadowWidth, shadowHeight, red, green, blue, aIntensity, dIntensity)
	, position_(glm::vec3(xPos, yPos, zPos))
	, constant_(con)
	, linear_(lin)
	, exponent_(exp)
	, farPlane_(far)
{
	// 构建投影矩阵: 点光源需要透视投影(90°视角形成立方体的 6个方向)
	float aspect = static_cast<float>(shadowWidth) / static_cast<float>(shadowHeight);		 // 计算贴图的宽高比 
	lightProj_ = glm::perspective(glm::radians(90.0f), aspect, near, far);	     // 构建投影矩阵

	// 创建用于全向阴影贴图的 ShadowMap
	shadowMap_ = new OmniShadowMap();
	shadowMap_->Init(shadowWidth, shadowHeight);
}

/// <summary>
/// 将点光源的属性传入 Shader（通过 uniform 变量）
/// </summary>
/// <param name="ambientIntensityLocation">环境光强度 uniform 位置</param>
/// <param name="ambientColourLocation">光颜色 uniform 位置</param>
/// <param name="diffuseIntensityLocation">漫反射强度 uniform 位置</param>
/// <param name="positionLocation">光源位置 uniform 位置</param>
/// <param name="constantLocation">衰减常数项 uniform 位置</param>
/// <param name="linearLocation">衰减线性项 uniform 位置</param>
/// <param name="exponentLocation">衰减二次项 uniform 位置</param>
void PointLight::UseLight(GLuint ambientIntensityLocation, GLuint ambientColourLocation, GLuint diffuseIntensityLocation,
						  GLuint positionLocation,GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation)
{
	glUniform3f(ambientColourLocation, colour_.x, colour_.y, colour_.z);
	glUniform1f(ambientIntensityLocation, ambientIntensity_);
	glUniform1f(diffuseIntensityLocation, diffuseIntensity_);

	glUniform3f(positionLocation, position_.x, position_.y, position_.z);
	glUniform1f(constantLocation, constant_);
	glUniform1f(linearLocation, linear_);
	glUniform1f(exponentLocation, exponent_);
}

/// <summary>
/// 计算立方体阴影映射所需的 6个光源视图变换矩阵
/// 每个方向对应一个面
/// </summary>
/// <returns>6个光源视图矩阵组成的数组</returns>
std::vector<glm::mat4> PointLight::CalculateLightTransform()
{
	std::vector<glm::mat4> lightMatrices;
	//+x, -x
	lightMatrices.push_back(lightProj_ * glm::lookAt(position_, position_ + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	lightMatrices.push_back(lightProj_ * glm::lookAt(position_, position_ + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));

	//+y, -y
	lightMatrices.push_back(lightProj_ * glm::lookAt(position_, position_ + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	lightMatrices.push_back(lightProj_ * glm::lookAt(position_, position_ + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));

	//+z, -z
	lightMatrices.push_back(lightProj_ * glm::lookAt(position_, position_ + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	lightMatrices.push_back(lightProj_ * glm::lookAt(position_, position_ + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

	return lightMatrices;
}

GLfloat PointLight::GetFarPlane()
{
	return farPlane_;
}
glm::vec3 PointLight::GetPosition()
{
	return position_;
}

/// <summary>
///析构函数
/// </summary>
PointLight::~PointLight()
{
}
