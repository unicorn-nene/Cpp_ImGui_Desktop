#include "Skybox.h"


/// <summary>
/// 构造函数
/// </summary>
Skybox::Skybox()
{
}

/// <summary>
/// 构造函数: 
/// 使用给定的六张立方体贴图创建天空盒，并初始化网格与着色器
/// </summary>
/// <param name="faceLocation">六张立方体贴图的路径的 vector</param>
Skybox::Skybox(std::vector<std::string> faceLocations)
{
	// 着色器初始化
	skyShader_ = new Shader();
	skyShader_->CreateFromFiles("Shaders/skybox.vert", "Shaders/skybox.frag");

	// 从 Shader 中获取 uniform 变量位置
	uniformProjection_ = skyShader_->GetProjectionLocation();
	uniformView_ = skyShader_->GetViewLocation();

	// 加载立方体贴图纹理
	glGenTextures(1, &textureId_);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId_);

	int width, height, bitDepth;

	// 遍历 6个面的纹理文件,加载到立方体贴图当中
	for (size_t i = 0; i < 6; ++i)
	{
		unsigned char* texData = stbi_load(faceLocations[i].c_str(), &width, &height, &bitDepth, 0);
		if (!texData)
		{
			printf("Failed to find: %s\n", faceLocations[i].c_str());
			return;
		}

		// 将图像数据上传到立方体贴图的对应面
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);

		stbi_image_free(texData);
	}

	// 设置立方体贴图的纹理参数
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 创建 skyBox 立方体 Mesh

	// 顶点索引数据
	unsigned int skyboxIndices[] = {
		// 前面
		0, 1, 2,
		2, 1, 3,
		// 右面
		2, 3, 5,
		5, 3, 7,
		// 后面
		5, 7, 4,
		4, 7, 6,
		// 左面
		4, 6, 0,
		0, 6, 1,
		// 顶面
		4, 0, 5,
		5, 0, 2,
		// 底面
		1, 6, 3,
		3, 6, 7
	};

	// 顶点坐标（每个顶点 8 个 float，但这里只用到位置坐标）
	float skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, -1.0f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, -1.0f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, -1.0f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,

		-1.0f,  1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f
	};

	skyMesh_ = new Mesh();
	skyMesh_->CreateMesh(skyboxVertices, skyboxIndices, 64, 36);
}


/// <summary>
/// 在当前 OpenGL 场景中绘制天空盒
/// 将深度写入暂时关闭，使用 view 和 projection 矩阵绘制立方体背景
/// </summary>
/// <param name="viewMatrix">当前摄像机的视图矩阵</param>
/// <param name="projectionMatrix">当前使用的投影矩阵</param>
void Skybox::DrawSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
	// 移除视图矩阵中的平移部分, 是天空盒始终跟随摄像机
	viewMatrix = glm::mat4(glm::mat3(viewMatrix));

	// 禁止写入深度缓冲区(防止天空盒遮挡物体)
	glDepthMask(GL_FALSE);

	skyShader_->UseShader();

	glUniformMatrix4fv(uniformProjection_, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView_, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId_);

	skyShader_->Validate();

	skyMesh_->RenderMesh();

	glDepthMask(GL_TRUE);
}


/// <summary>
/// 构造函数
/// </summary>
Skybox::~Skybox()
{
}
