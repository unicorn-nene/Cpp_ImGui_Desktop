#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

#include <GL\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "CommonValues.h"

#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

/// <summary>
/// Shader 类负责加载, 编译, 链接 OpenGL着色器程序, 并获取/更新着色器中使用的 uniform 变量位置.
/// </summary>
class Shader
{
public:
	Shader();

	void CreateFromString(const char* vertexCode, const char* fragmentCode);
	void CreateFromFiles(const char* vertexLocation, const char* fragmentLocation);
	void CreateFromFiles(const char* vertexLocation, const char* geometryLocation, const char* fragmentLocation);

	void Validate();

	std::string ReadFile(const char* fileLocation);

	void SetDirectionalLight(DirectionalLight* dLight);
	void SetPointLights(PointLight* pLight, unsigned int lightCount, unsigned int textureUnit, unsigned int offset);
	void SetSpotLights(SpotLight* sLight, unsigned int lightCount, unsigned int textureUnit, unsigned int offset);
	void SetTexture(GLuint textureUnit);
	void SetDirectionalShadowMap(GLuint textureUnit);
	void SetDirectionalLightTransform(glm::mat4* lTransform);
	void SetLightMatrices(std::vector<glm::mat4> lightMatrices);

	void UseShader();
	void ClearShader();

	~Shader();

#pragma region uniform_getter
	GLuint GetProjectionLocation();
	GLuint GetModelLocation();
	GLuint GetViewLocation();
	GLuint GetAmbientIntensityLocation();
	GLuint GetAmbientColourLocation();
	GLuint GetDiffuseIntensityLocation();
	GLuint GetDirectionLocation();
	GLuint GetSpecularIntensityLocation();
	GLuint GetShininessLocation();
	GLuint GetEyePositionLocation();
	GLuint GetOmniLightPosLocation();
	GLuint GetFarPlaneLocation();
#pragma endregion

private:
#pragma region private function
	void CompileShader(const char* vertexCode, const char* fragmentCode);
	void CompileShader(const char* vertexCode, const char* geometryCode, const char* fragmentCode);
	void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType);

	void CompileProgram();
#pragma endregion

private:
#pragma region light_count
	int pointLightCount_{};								///< 当前点光源数量
	int spotLightCount_{};								///< 当前聚光灯数量
#pragma endregion

	GLuint shaderID_{};									///< 当前着色器程序对象的 OpenGL ID

#pragma region uniform
	GLuint uniformProjection_{};						///< 投影矩阵 uniform 位置
	GLuint uniformModel_{};								///< 模型矩阵 uniform 位置
	GLuint uniformView_{};								///< 视图矩阵 uniform 位置
	GLuint uniformEyePosition_{};						///< 摄像机位置 uniform 位置
	GLuint uniformSpecularIntensity_{};					///< 镜面反射强度 uniform 
	GLuint uniformShininess_{};							///< 镜面高光系数 uniform 
	GLuint uniformTexture_{};							///< 纹理 uniform 
	GLuint uniformDirectionalShadowMap_{};				///< 平行光阴影贴图 sampler uniform 
	GLuint uniformDirectionalLightTransform_{};			///< 平行光阴影变换矩阵 uniform
	GLuint uniformOmniLightPos_{};						///< 点光源阴影贴图光源位置 uniform
	GLuint uniformFarPlane_{};							///< 点光源阴影贴图远裁面 uniform

	GLuint uniformLightMatrices_[6]{};					///< 立方体阴影贴图对应 6 个视角的光源变换矩阵 uniform（点光源阴影）
#pragma endregion

#pragma region uniformDirectionalLight_struct
	///< 平行光结构体
	struct
	{
		GLuint uniformColour_{};						///< 光颜色
		GLuint uniformAmbientIntensity_{};				///< 环境光强度
		GLuint uniformDiffuseIntensity_{};				///< 漫反射强度

		GLuint uniformDirection_{};						///< 光照方向
	}uniformDirectionalLight_;
#pragma endregion

#pragma region uniformPointLight_struct
	GLuint uniformPointLightCount_{};					///< 点光源数量 uniform
	///< 点光源(结构体)数组（支持多光源） uniform 参数集合
	struct
	{
		GLuint uniformColour_{};						///< 光颜色
		GLuint uniformAmbientIntensity_{};				///< 环境光强度
		GLuint uniformDiffuseIntensity_{};				///< 漫反射强度

		GLuint uniformPosition_{};						///< 光源位置
		GLuint uniformConstant_{};						///< 衰减常数项
		GLuint uniformLinear_{};						///< 衰减一次项
		GLuint uniformExponent_{};						///< 衰减二次项
	} uniformPointLight_[MAX_POINT_LIGHTS];
#pragma endregion

#pragma region uniformSpotLight_struct
	GLuint uniformSpotLightCount_{};					///< 聚光灯数量 uniform
	///< 聚光灯(结构体)数组（支持多光源） uniform 参数集合
	struct
	{
		GLuint uniformColour_{};						///< 光颜色
		GLuint uniformAmbientIntensity_{};				///< 环境光强度
		GLuint uniformDiffuseIntensity_{};				///< 漫反射强度

		GLuint uniformPosition_{};						///< 光源位置
		GLuint uniformConstant_{};						///< 衰减常数项
		GLuint uniformLinear_{};						///< 衰减一次项
		GLuint uniformExponent_{};						///< 衰减二次项

		GLuint uniformDirection_{};						///< 聚光方向
		GLuint uniformEdge_{};							///< 聚光边缘衰减角（cos(θ)）
	}uniformSpotLight_[MAX_SPOT_LIGHTS];
#pragma endregion

#pragma region uniformOmniShadowMap_struct
	///< 立体阴影贴图(结构体) uniform 集合
	struct
	{
		GLuint shadowMap_{};							///< 立体阴影贴图样本 uniform
		GLuint farPlane_{};								///< 阴影采样中的最大深度范围
	}uniformOmniShadowMap_[MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS];
#pragma endregion
};

