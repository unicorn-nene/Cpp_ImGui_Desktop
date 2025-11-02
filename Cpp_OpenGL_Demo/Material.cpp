#include "Material.h"

/// <summary>
/// 默认构造函数, 将镜面强度与光泽度初始化为 0，表示无高光反射. 
/// </summary>
Material::Material()
	:specularIntensity_(0.0f)
	,shininess_(0.0f)
{

}

/// <summary>
/// 构造函数,使用给定的镜面反射强度和光泽度初始化材质
/// </summary>
/// <param name="sIntensity">镜面反射强度</param>
/// <param name="shine">光泽度</param>
Material::Material(GLfloat sIntensity, GLfloat shine)
	:specularIntensity_(sIntensity)
	,shininess_(shine)
{

}

/// <summary>
/// 向 Shader 传递材质的镜面反射强度和光泽度
/// </summary>
/// <param name="specularIntensityLocation">Shader 镜面反射强度 uniform 位置</param>
/// <param name="shininessLocation">Shader 光泽度 uniform 位置</param>
void Material::UseMaterial(GLuint specularIntensityLocation, GLuint shininessLocation)
{
	glUniform1f(specularIntensityLocation, specularIntensity_);
	glUniform1f(shininessLocation, shininess_);
}

/// <summary>
/// 析构函数
/// </summary>
Material::~Material()
{
}
