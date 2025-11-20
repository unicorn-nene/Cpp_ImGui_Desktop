#pragma once

#include <vector>
#include <string>

#include <GL\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "CommonValues.h"

#include "Mesh.h"
#include "Shader.h"

/// <summary>
/// Skybox 类用于在 3D 场景中创建和渲染天空盒（天空背景立方体）.
/// 它会加载六张纹理图（立方体贴图），并使用一个特殊的着色器在场景背景中绘制
/// </summary>
class Skybox
{
public:
	Skybox();

	Skybox(std::vector<std::string> faceLocations);

	void DrawSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);

	~Skybox();

private:
	Mesh* skyMesh_{};					 ///< 天空盒 Mesh
	Shader* skyShader_{};				 ///< 天空盒 Shader

	GLuint textureId_{};				 ///< 纹理 ID
	GLuint uniformProjection_{};		 ///< 投影矩阵
	GLuint uniformView_{};				 ///< 摄像机的视图矩阵
};

