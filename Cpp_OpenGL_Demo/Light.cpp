#include "Light.h"

/// <summary>
/// 默认构造函数，初始化白色光源，无阴影贴图。
/// </summary>
Light::Light()
	:colour_(glm::vec3(1.0f, 1.0f, 1.0f))
	,ambientIntensity_(1.0f)
	,diffuseIntensity_(0.0f)
{

}

/// <summary>
/// 创建可用于阴影渲染的光源
/// </summary>
/// <param name="shadowWidth">阴影贴图宽度</param>
/// <param name="shadowHeight">阴影贴图高度</param>
/// <param name="red">光源颜色 R 分量</param>
/// <param name="green">光源颜色 G 分量</param>
/// <param name="blue">光源颜色 B 分量</param>
/// <param name="aIntensity">环境光分量强度</param>
/// <param name="dIntensity">漫反射光分量强度</param>
Light::Light(GLuint shadowWidth, GLuint shadowHeight,
			 GLfloat red, GLfloat green, GLfloat blue,
			 GLfloat aIntensity, GLfloat dIntensity)
	:colour_(glm::vec3(red, green, blue))
	,ambientIntensity_(aIntensity)
	,diffuseIntensity_(dIntensity)
{
	shadowMap_ = new ShadowMap();
	shadowMap_->Init(shadowWidth, shadowHeight);
}

/// <summary>
/// 析构函数
/// </summary>
Light::~Light()
{
}
