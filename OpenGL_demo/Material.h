#pragma once

#include <GL\glew.h>

/// <summary>
/// 表示一个材质（Material）对象，定义了物体表面在高光（specular）阶段的反射特性。
/// 该类主要用于在 Phong 光照模型中设置物体的“镜面反射强度”和“光泽度”
/// </summary>
class Material
{
public:
	Material();
	Material(GLfloat sIntensity, GLfloat shine);

	void UseMaterial(GLuint specularIntensityLocation, GLuint shininessLocation);

	~Material();

private: 
	GLfloat specularIntensity_{};		///< 材质镜面反射强度, 控制反射光的强度,值越大越亮
	GLfloat shininess_{};				///< 材质光泽度, 控制高光的集中程度,值越大高光区域越小且越锐利.
};

