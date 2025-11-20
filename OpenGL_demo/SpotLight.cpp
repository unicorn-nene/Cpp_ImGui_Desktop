#include "SpotLight.h"

/// <summary>
/// 默认构造函数
/// 初始化为 关闭状态, 初始光线方向正下方, 无衰减, 白色光.
/// </summary>
SpotLight::SpotLight() 
	: PointLight()
	, direction_(glm::vec3(0.0f, -1.0f, 0.0f))
	, edge_(0.0f)
	, procEdge_(cosf(glm::radians(edge_)))
	, isOn_(true)
{
}

/// <summary>
/// 构造函数：创建具有阴影贴图支持的聚光灯。
/// </summary>
/// <param name="shadowWidth">阴影贴图的宽度（像素）</param>
/// <param name="shadowHeight">阴影贴图的高度（像素）</param>
/// <param name="near">透视投影的近平面</param>
/// <param name="far">透视投影的远平面</param>
/// <param name="red">光的红色分量 R（0~1）</param>
/// <param name="green">光的绿色分量 G（0~1）</param>
/// <param name="blue">光的蓝色分量 B（0~1）</param>
/// <param name="aIntensity">环境光强度</param>
/// <param name="dIntensity">漫反射光强度</param>
/// <param name="xPos">光源位置的 X 坐标/param>
/// <param name="yPos">光源位置的 Y 坐标</param>
/// <param name="zPos">光源位置的 Z 坐标</param>
/// <param name="xDir">光线方向向量的 X 分量</param>
/// <param name="yDir">光线方向向量的 Y 分量</param>
/// <param name="zDir">光线方向向量的 Z 分量</param>
/// <param name="con">常数衰减系数（constant）</param>
/// <param name="lin">线性衰减系数（linear）</param>
/// <param name="exp">指数衰减系数（exponent）</param>
/// <param name="edg">光照锥边缘角（单位：度）, 角度越小，光束越集中</param>
SpotLight::SpotLight(GLuint shadowWidth, GLuint shadowHeight,
					 GLfloat near, GLfloat far, 
					 GLfloat red, GLfloat green, GLfloat blue,
					 GLfloat aIntensity, GLfloat dIntensity, 
					 GLfloat xPos, GLfloat yPos, GLfloat zPos, 
					 GLfloat xDir, GLfloat yDir, GLfloat zDir, 
					 GLfloat con, GLfloat lin, GLfloat exp, 
					 GLfloat edg)
	: PointLight(shadowWidth, shadowHeight, near, far, red, green, blue, aIntensity, dIntensity, xPos, yPos, zPos, con, lin, exp)
	, direction_(glm::normalize(glm::vec3(xDir, yDir, zDir)))
	, edge_(edg)
	, procEdge_(cosf(glm::radians(edge_)))
	, isOn_(true)
{

}

/// <summary>
/// 将聚光灯光照参数传递给着色器程序, 用于在渲染阶段绑定聚光灯的所有 Uniform。
/// 根据灯光是否开启（isOn_），动态设置环境光与漫反射光强度
/// </summary>
/// <param name="ambientIntensityLocation">Uniform - 环境光强度位置</param>
/// <param name="ambientColourLocation">Uniform - 环境光颜色位置</param>
/// <param name="diffuseIntensityLocation">Uniform - 漫反射强度位置</param>
/// <param name="positionLocation">Uniform - 光源位置位置</param>
/// <param name="directionLocation">Uniform - 光线方向位置</param>
/// <param name="constantLocation">Uniform - 常数衰减参数位置</param>
/// <param name="linearLocation">Uniform - 线性衰减参数位置</param>
/// <param name="exponentLocation">Uniform - 指数衰减参数位置</param>
/// <param name="edgeLocation">Uniform - 聚光锥边缘角（余弦形式）位置</param>
void SpotLight::UseLight(GLuint ambientIntensityLocation, GLuint ambientColourLocation, 
						 GLuint diffuseIntensityLocation, GLuint positionLocation, GLuint directionLocation, 
						 GLuint constantLocation, GLuint linearLocation, GLuint exponentLocation, 
						 GLuint edgeLocation)
{
	glUniform3f(ambientColourLocation, colour_.x, colour_.y, colour_.z);

	// 如果灯光开启, 使用正常光照强度
	if (isOn_)
	{
		glUniform1f(ambientIntensityLocation, ambientIntensity_);
		glUniform1f(diffuseIntensityLocation, diffuseIntensity_);
	}
	else // 灯光关闭, 关闭(聚光灯)灯光效果
	{
		glUniform1f(ambientIntensityLocation, 0.0f);
		glUniform1f(diffuseIntensityLocation, 0.0f);
	}

	// 传递光源的空间位置与衰减参数（控制光强随距离衰减）
	glUniform3f(positionLocation, position_.x, position_.y, position_.z);
	glUniform1f(constantLocation, constant_);
	glUniform1f(linearLocation, linear_);
	glUniform1f(exponentLocation, exponent_);

	// 传递光照方向与边缘衰减（通过余弦预计算）
	glUniform3f(directionLocation, direction_.x, direction_.y, direction_.z);
	glUniform1f(edgeLocation, procEdge_);
}

/// <summary>
/// 更新聚光灯的位置与方向, 可设置为跟随 摄像机移动
/// </summary>
/// <param name="pos"></param>
/// <param name="dir"></param>
void SpotLight::SetFlash(glm::vec3 pos, glm::vec3 dir)
{
	position_ = pos;
	direction_ = dir;
}

/// <summary>
/// 析构函数
/// </summary>
SpotLight::~SpotLight()
{
}
