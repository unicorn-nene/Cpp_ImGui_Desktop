#include "Shader.h"

/// <summary>
/// 默认构造函数，初始化所有 uniform 位置及 Shader 程序 ID 为 0。
/// </summary>
Shader::Shader()
{
}

/// <summary>
/// 使用顶点着色器和片段着色器源码字符串创建 Shader 程序
/// </summary>
/// <param name="vertexCode">顶点着色器 GLSL 源码</param>
/// <param name="fragmentCode">片段着色器 GLSL 源码</param>
void Shader::CreateFromString(const char* vertexCode, const char* fragmentCode)
{
	CompileShader(vertexCode, fragmentCode);
}

/// <summary>
/// 使用顶点、几何和片段着色器文件路径创建 Shader 程序。
/// </summary>
/// <param name="vertexLocation">顶点着色器文件路径</param>
/// <param name="geometryLocation">几何着色器文件路径</param>
/// <param name="fragmentLocation">片段着色器文件路径</param>
void Shader::CreateFromFiles(const char* vertexLocation, const char* fragmentLocation)
{
	std::string vertexString = ReadFile(vertexLocation);
	std::string fragmentString = ReadFile(fragmentLocation);
	const char* vertexCode = vertexString.c_str();
	const char* fragmentCode = fragmentString.c_str();

	CompileShader(vertexCode, fragmentCode);
}

/// <summary>
/// 使用顶点、几何和片段着色器文件路径创建 Shader 程序。
/// </summary>
/// <param name="vertexLocation">顶点着色器文件路径</param>
/// <param name="geometryLocation">几何着色器文件路径</param>
/// <param name="fragmentLocation">片段着色器文件路径</param>
void Shader::CreateFromFiles(const char* vertexLocation, const char* geometryLocation, const char* fragmentLocation)
{
	std::string vertexString = ReadFile(vertexLocation);
	std::string geometryString = ReadFile(geometryLocation);
	std::string fragmentString = ReadFile(fragmentLocation);
	const char* vertexCode = vertexString.c_str();
	const char* geometryCode = geometryString.c_str();
	const char* fragmentCode = fragmentString.c_str();

	CompileShader(vertexCode, geometryCode, fragmentCode);
}

/// <summary>
/// 验证 Shader 程序的链接状态与有效性。
/// </summary>
void Shader::Validate()
{
	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glValidateProgram(shaderID_);
	glGetProgramiv(shaderID_, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shaderID_, sizeof(eLog), NULL, eLog);
		printf("Error validating program: '%s'\n", eLog);
		return;
	}
}

/// <summary>
/// 读取着色器文件并返回其内容字符串
/// </summary>
/// <param name="fileLocation">文件路径</param>
/// <returns>文件完整内容字符串</returns>
std::string Shader::ReadFile(const char* fileLocation)
{
	std::string content;
	std::ifstream fileStream(fileLocation, std::ios::in);

	if (!fileStream.is_open()) {
		printf("Failed to read %s! File doesn't exist.", fileLocation);
		return "";
	}

	std::string line = "";
	while (!fileStream.eof())
	{
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

#pragma region  uniform getter
GLuint Shader::GetProjectionLocation()
{
	return uniformProjection_;
}
GLuint Shader::GetModelLocation()
{
	return uniformModel_;
}
GLuint Shader::GetViewLocation()
{
	return uniformView_;
}
GLuint Shader::GetAmbientColourLocation()
{
	return uniformDirectionalLight_.uniformColour_;
}
GLuint Shader::GetAmbientIntensityLocation()
{
	return uniformDirectionalLight_.uniformAmbientIntensity_;
}
GLuint Shader::GetDiffuseIntensityLocation()
{
	return uniformDirectionalLight_.uniformDiffuseIntensity_;
}
GLuint Shader::GetDirectionLocation()
{
	return uniformDirectionalLight_.uniformDirection_;
}
GLuint Shader::GetSpecularIntensityLocation()
{
	return uniformSpecularIntensity_;
}
GLuint Shader::GetShininessLocation()
{
	return uniformShininess_;
}
GLuint Shader::GetEyePositionLocation()
{
	return uniformEyePosition_;
}
GLuint Shader::GetOmniLightPosLocation()
{
	return uniformOmniLightPos_;
}
GLuint Shader::GetFarPlaneLocation()
{
	return uniformFarPlane_;
}
#pragma endregion

/// <summary>
/// 将 directionalLight(平行光)属性传入 Shader的 uniform 
/// </summary>
/// <param name="dLight">要传入的 directionalLight对象指针</param>
void Shader::SetDirectionalLight(DirectionalLight * dLight)
{
	dLight->UseLight(uniformDirectionalLight_.uniformAmbientIntensity_, uniformDirectionalLight_.uniformColour_,
		uniformDirectionalLight_.uniformDiffuseIntensity_, uniformDirectionalLight_.uniformDirection_);
}
/// <summary>
/// 将多个 PointLight (属性)及其阴影贴图传入 Shader 的 uniform 
/// </summary>
/// <param name="pLight">指向 PointLight 数组的指针</param>
/// <param name="lightCount">点光源数量</param>
/// <param name="textureUnit">起始纹理单元索引（用于绑定阴影贴图）</param>
/// <param name="offset">阴影贴图在 Uniform 数组中的偏移量（用于区分不同光源类型）</param>
void Shader::SetPointLights(PointLight * pLight, unsigned int lightCount, unsigned int textureUnit, unsigned int offset)
{
	if (lightCount > MAX_POINT_LIGHTS) lightCount = MAX_POINT_LIGHTS;

	glUniform1i(uniformPointLightCount_, lightCount);

	for (size_t i = 0; i < lightCount; i++)
	{
		pLight[i].UseLight(uniformPointLight_[i].uniformAmbientIntensity_, uniformPointLight_[i].uniformColour_,
			uniformPointLight_[i].uniformDiffuseIntensity_, uniformPointLight_[i].uniformPosition_,
			uniformPointLight_[i].uniformConstant_, uniformPointLight_[i].uniformLinear_, uniformPointLight_[i].uniformExponent_);

		pLight[i].getShadowMap()->Read(GL_TEXTURE0 + textureUnit + i);
		glUniform1i(uniformOmniShadowMap_[i + offset].shadowMap_, textureUnit + i);
		glUniform1f(uniformOmniShadowMap_[i + offset].farPlane_, pLight[i].GetFarPlane());
	}
}

/// <summary>
/// 将多个聚光灯（SpotLight）的属性和对应阴影贴图信息传入 Shader 的 uniform
/// </summary>
/// <param name="sLight">指向 SpotLight 数组的指针</param>
/// <param name="lightCount">聚光灯数量</param>
/// <param name="textureUnit">起始纹理单元索引（用于绑定阴影贴图</param>
/// <param name="offset">阴影贴图 uniform 索引偏移量（防止与其他光源类型冲突</param>
void Shader::SetSpotLights(SpotLight * sLight, unsigned int lightCount, unsigned int textureUnit, unsigned int offset)
{
	if (lightCount > MAX_SPOT_LIGHTS) lightCount = MAX_SPOT_LIGHTS;

	glUniform1i(uniformSpotLightCount_, lightCount);

	for (size_t i = 0; i < lightCount; i++)
	{
		sLight[i].UseLight(uniformSpotLight_[i].uniformAmbientIntensity_, uniformSpotLight_[i].uniformColour_,
			uniformSpotLight_[i].uniformDiffuseIntensity_, uniformSpotLight_[i].uniformPosition_, uniformSpotLight_[i].uniformDirection_,
			uniformSpotLight_[i].uniformConstant_, uniformSpotLight_[i].uniformLinear_, uniformSpotLight_[i].uniformExponent_,
			uniformSpotLight_[i].uniformEdge_);

		sLight[i].getShadowMap()->Read(GL_TEXTURE0 + textureUnit + i);
		glUniform1i(uniformOmniShadowMap_[i + offset].shadowMap_, textureUnit + i);
		glUniform1f(uniformOmniShadowMap_[i + offset].farPlane_, sLight[i].GetFarPlane());
	}
}

/// <summary>
/// 绑定并设置场景主纹理（通常是物体的漫反射纹理）
/// </summary>
/// <param name="textureUnit">主纹理单元索引</param>
void Shader::SetTexture(GLuint textureUnit)
{
	glUniform1i(uniformTexture_, textureUnit);
}

/// <summary>
/// 设置平行光阴影贴图的纹理单元索引。
/// </summary>
/// <param name="textureUnit">阴影贴图纹理单元索引</param>
void Shader::SetDirectionalShadowMap(GLuint textureUnit)
{
	glUniform1i(uniformDirectionalShadowMap_, textureUnit);
}

/// <summary>
/// 设置平行光的光照变换矩阵（light transform），用于阴影贴图的深度计算
/// </summary>
/// <param name="lTransform">指向光照变换矩阵（通常为光源视图投影矩阵）的指针</param>
void Shader::SetDirectionalLightTransform(glm::mat4* lTransform)
{
	glUniformMatrix4fv(uniformDirectionalLightTransform_, 1, GL_FALSE, glm::value_ptr(*lTransform));
}

/// <summary>
/// 设置点光源或聚光灯的六个立方体阴影变换矩阵（用于 omni-directional shadow mapping）  
/// 每个矩阵对应立方体贴图的一个面 (+X, -X, +Y, -Y, +Z, -Z)
/// </summary>
/// <param name="lightMatrices">包含六个视图投影矩阵的 vector</param>
void Shader::SetLightMatrices(std::vector<glm::mat4> lightMatrices)
{
	for (size_t i = 0; i < 6; i++)
	{
		glUniformMatrix4fv(uniformLightMatrices_[i], 1, GL_FALSE, glm::value_ptr(lightMatrices[i]));
	}
}

/// <summary>
/// 启用当前着色器程序
/// </summary>
void Shader::UseShader()
{
	glUseProgram(shaderID_);
}

/// <summary>
/// 清理着色器资源
/// </summary>
void Shader::ClearShader()
{
	if (shaderID_ != 0)
	{
		glDeleteProgram(shaderID_);
		shaderID_ = 0;
	}

	uniformModel_ = 0;
	uniformProjection_ = 0;
}

/// <summary>
/// 析构函数
/// </summary>
Shader::~Shader()
{
	ClearShader();
}

/// <summary>
/// 编译顶点着色器, 片段着色器, 并链接为 Shader 程序
/// </summary>
/// <param name="vertexCode">顶点着色器 GLSL 源码</param>
/// <param name="fragmentCode">片段着色器 GLSL 源码</param>
void Shader::CompileShader(const char* vertexCode, const char* fragmentCode)
{
	shaderID_ = glCreateProgram();

	if (!shaderID_)
	{
		printf("Error creating shader program!\n");
		return;
	}

	AddShader(shaderID_, vertexCode, GL_VERTEX_SHADER);
	AddShader(shaderID_, fragmentCode, GL_FRAGMENT_SHADER);

	CompileProgram();
}

/// <summary>
/// 编译顶点、几何与片段着色器源码，并链接为 Shader 程序。
/// </summary>
/// <param name="vertexCode">顶点着色器 GLSL 源码</param>
/// <param name="geometryCode">几何着色器 GLSL 源码</param>
/// <param name="fragmentCode">片段着色器 GLSL 源码</param>
void Shader::CompileShader(const char* vertexCode, const char* geometryCode, const char* fragmentCode)
{
	shaderID_ = glCreateProgram();

	if (!shaderID_)
	{
		printf("Error creating shader program!\n");
		return;
	}

	AddShader(shaderID_, vertexCode, GL_VERTEX_SHADER);
	AddShader(shaderID_, geometryCode, GL_GEOMETRY_SHADER);
	AddShader(shaderID_, fragmentCode, GL_FRAGMENT_SHADER);

	CompileProgram();
}

/// <summary>
/// 将单个 Shader 源码添加到着色器程序中进行编译
/// </summary>
/// <param name="theProgram">目标 shader程序</param>
/// <param name="shaderCode">要编译的着色器 GLSL 源码</param>
/// <param name="shaderType">要编译的着色器类型</param>
void Shader::AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
	GLuint theShader = glCreateShader(shaderType);

	const GLchar* theCode[1];
	theCode[0] = shaderCode;

	GLint codeLength[1];
	codeLength[0] = strlen(shaderCode);

	glShaderSource(theShader, 1, theCode, codeLength);
	glCompileShader(theShader);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(theShader, sizeof(eLog), NULL, eLog);
		printf("Error compiling the %d shader: '%s'\n", shaderType, eLog);
		return;
	}

	glAttachShader(theProgram, theShader);
}

/// <summary>
/// 链接所有的已编译的 着色器(Shader) 模块到 最终 Shader 程序
/// </summary>
void Shader::CompileProgram() {

	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glLinkProgram(shaderID_);
	glGetProgramiv(shaderID_, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shaderID_, sizeof(eLog), NULL, eLog);
		printf("Error linking program: '%s'\n", eLog);
		return;
	}

	uniformProjection_ = glGetUniformLocation(shaderID_, "projection");
	uniformModel_ = glGetUniformLocation(shaderID_, "model");
	uniformView_ = glGetUniformLocation(shaderID_, "view");
	uniformDirectionalLight_.uniformColour_ = glGetUniformLocation(shaderID_, "directionalLight.base.colour");
	uniformDirectionalLight_.uniformAmbientIntensity_ = glGetUniformLocation(shaderID_, "directionalLight.base.ambientIntensity");
	uniformDirectionalLight_.uniformDirection_ = glGetUniformLocation(shaderID_, "directionalLight.direction");
	uniformDirectionalLight_.uniformDiffuseIntensity_ = glGetUniformLocation(shaderID_, "directionalLight.base.diffuseIntensity");
	uniformSpecularIntensity_ = glGetUniformLocation(shaderID_, "material.specularIntensity");
	uniformShininess_ = glGetUniformLocation(shaderID_, "material.shininess");
	uniformEyePosition_ = glGetUniformLocation(shaderID_, "eyePosition");

	uniformPointLightCount_ = glGetUniformLocation(shaderID_, "pointLightCount");

	for (size_t i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		char locBuff[100] = { '\0' };

		snprintf(locBuff, sizeof(locBuff), "pointLights[%d].base.colour", i);
		uniformPointLight_[i].uniformColour_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "pointLights[%d].base.ambientIntensity", i);
		uniformPointLight_[i].uniformAmbientIntensity_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "pointLights[%d].base.diffuseIntensity", i);
		uniformPointLight_[i].uniformDiffuseIntensity_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "pointLights[%d].position", i);
		uniformPointLight_[i].uniformPosition_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "pointLights[%d].constant", i);
		uniformPointLight_[i].uniformConstant_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "pointLights[%d].linear", i);
		uniformPointLight_[i].uniformLinear_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "pointLights[%d].exponent", i);
		uniformPointLight_[i].uniformExponent_ = glGetUniformLocation(shaderID_, locBuff);
	}

	uniformSpotLightCount_ = glGetUniformLocation(shaderID_, "spotLightCount");

	for (size_t i = 0; i < MAX_SPOT_LIGHTS; i++)
	{
		char locBuff[100] = { '\0' };

		snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.base.colour", i);
		uniformSpotLight_[i].uniformColour_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.base.ambientIntensity", i);
		uniformSpotLight_[i].uniformAmbientIntensity_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.base.diffuseIntensity", i);
		uniformSpotLight_[i].uniformDiffuseIntensity_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.position", i);
		uniformSpotLight_[i].uniformPosition_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.constant", i);
		uniformSpotLight_[i].uniformConstant_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.linear", i);
		uniformSpotLight_[i].uniformLinear_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.exponent", i);
		uniformSpotLight_[i].uniformExponent_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "spotLights[%d].direction", i);
		uniformSpotLight_[i].uniformDirection_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "spotLights[%d].edge", i);
		uniformSpotLight_[i].uniformEdge_ = glGetUniformLocation(shaderID_, locBuff);
	}

	uniformDirectionalLightTransform_ = glGetUniformLocation(shaderID_, "directionalLightTransform");
	uniformTexture_ = glGetUniformLocation(shaderID_, "theTexture");
	uniformDirectionalShadowMap_ = glGetUniformLocation(shaderID_, "directionalShadowMap");

	uniformOmniLightPos_ = glGetUniformLocation(shaderID_, "lightPos");
	uniformFarPlane_ = glGetUniformLocation(shaderID_, "farPlane");

	for (size_t i = 0; i < 6; i++)
	{
		char locBuff[100] = { '\0' };

		snprintf(locBuff, sizeof(locBuff), "lightMatrices[%d]", i);
		uniformLightMatrices_[i] = glGetUniformLocation(shaderID_, locBuff);
	}

	for (size_t i = 0; i < MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS; i++)
	{
		char locBuff[100] = { '\0' };

		snprintf(locBuff, sizeof(locBuff), "omniShadowMaps[%d].shadowMap", i);
		uniformOmniShadowMap_[i].shadowMap_ = glGetUniformLocation(shaderID_, locBuff);

		snprintf(locBuff, sizeof(locBuff), "omniShadowMaps[%d].farPlane", i);
		uniformOmniShadowMap_[i].farPlane_ = glGetUniformLocation(shaderID_, locBuff);
	}
}
