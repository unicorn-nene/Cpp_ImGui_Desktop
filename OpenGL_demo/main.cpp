#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "CommonValues.h"

#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

#include "Model.h"

#include "Skybox.h"

#pragma region gaboal_value

#pragma region 数学常量
const float toRadians = 3.14159265f / 180.0f;	///< 1 弧度 = Π / 180
#pragma endregion

#pragma region shader_uniform句柄
GLuint uniformProjection{};						    ///< 投影矩阵 uniform 位置
GLuint uniformModel{};							    ///< 模型矩阵 uniform 位置
GLuint uniformView{};							    ///< 视图矩阵 uniform 位置
GLuint uniformEyePosition{};					    ///< 相机位置 uniform 位置
GLuint uniformSpecularIntensity{};				    ///< 镜面反射强度 uniform 位置
GLuint uniformShininess{};						    ///< 高光系数 uniform 位置
GLuint uniformDirectionalLightTransform = 0;	    ///< 平行光变换矩阵 uniform 位置
GLuint uniformOmniLightPos = 0;					    ///< 点光源位置 uniform 位置
GLuint uniformFarPlane = 0;						    ///< 远裁剪平面 uniform 位置
#pragma endregion		

#pragma region Window and Scene
Window mainWindow{};							    ///< 主窗口对象
std::vector<Mesh*> meshList{};					    ///< Mesh对象指针容器列表(vector)

std::vector<Shader> shaderList{};				    ///< Shader 对象列表
Shader directionalShadowShader{};				    ///< 平行光阴影 Shader
Shader omniShadowShader{};						    ///< 点光源阴影 Shader

Camera camera{};								    ///< 相机对象
#pragma endregion							  

#pragma region Texture and materia
Texture brickTexture{};							    ///< 砖块纹理
Texture dirtTexture{};							    ///< 泥土纹理
Texture plainTexture{};							    ///< 普通纹理

Material shinyMaterial{};						    ///< 闪亮材质
Material dullMaterial{};						    ///< 暗淡材质
#pragma endregion	

#pragma region 外部 Model
Model xwing{};									    ///< x-wing 战机 Model
Model blackhawk{};								    ///< 黑鹰直升机 Model
#pragma endregion									 

#pragma region 光源
DirectionalLight mainLight{};					    ///< 主平行光(源)
PointLight pointLights[MAX_POINT_LIGHTS];		    ///< 点光源阵列
SpotLight spotLights[MAX_SPOT_LIGHTS];			    ///< 聚光灯阵列
#pragma endregion

#pragma region 天空盒
Skybox skybox;									    ///< 天空盒对象
#pragma endregion

#pragma region 点光源计数
unsigned int pointLightCount = 0;				    ///< 当前点光源数量
unsigned int spotLightCount = 0;				    ///< 当前聚光灯数量
#pragma endregion

#pragma region 时间
GLfloat deltaTime = 0.0f;						    ///< Δ时间, 每帧之间时间差
GLfloat lastTime = 0.0f;						    ///< 上一帧时间
#pragma endregion

#pragma region Model Status
GLfloat blackhawkAngle = 0.0f;					    ///< 黑鹰直升机旋转角度
#pragma endregion

#pragma region Shader Path
static const char* vShader = "Shaders/shader.vert"; ///< 顶点着色器文件路径
static const char* fShader = "Shaders/shader.frag"; ///< 片段着色器文件路径
#pragma endregion

#pragma endregion

#pragma region pre-function

/// <summary>
/// 为给定顶点数组和索引数组计算平均法线（平滑法线)
/// </summary>
/// <param name="indices">顶点数组</param>
/// <param name="indiceCount">顶点数量</param>
/// <param name="vertices">索引数组</param>
/// <param name="verticeCount">索引数量</param>
/// <param name="vLength">每个顶点的 float数量</param>
/// <param name="normalOffset">法线(nx,ny,nz)在顶点内部的偏移</param>
void calcAverageNormals(unsigned int * indices, unsigned int indiceCount, GLfloat * vertices, unsigned int verticeCount, 
						unsigned int vLength, unsigned int normalOffset)
{	// 顶点结构: x, y, z,		u, v,		nx, ny, nz

	// 对每个三角形计算面法线并累加到三个顶点的法线分量
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;

		// v1 = p1 - p0, v2 = p2 - p0
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 1]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 1]);

		// 面法线 = cross(v1, v2)
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		// 将计算得到的面法线累加到每个顶点的法线位置
		in0 += normalOffset;
		in1 += normalOffset;
		in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	// 归一化每一个顶点的法线, 使其成为单位向量
	for (size_t i = 0; i < verticeCount / vLength; ++i)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);

		vertices[nOffset] = vec.x;
		vertices[nOffset + 1] = vec.y;
		vertices[nOffset + 2] = vec.z;
	}
}

/// <summary>
/// 创建 Mesh 实例, 这里创建了两个四面体和一个地面立方体 Mesh
/// </summary>
void CreateObjects() 
{
	// 四面体的索引数组
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	// 四面体的顶点数组
	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
		-1.0f, -1.0f, -0.6f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		 0.0f, -1.0f,  1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		 0.0f,  1.0f,  0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	// 地板 Mesh 的索引数组
	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};
	// 地板 mesh 的顶点数组
	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		 10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f,  10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		 10.0f, 0.0f,  10.0f,	10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	// 计算四面体平均法线
	calcAverageNormals(indices, 12, vertices, 32, 8, 5);

	// 创建 Mesh 并加入 MeshList
	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);
}

/// <summary>
/// 创建并加载各类着色器（场景着色器 + 阴影着色器）
/// </summary>
void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	// 平行光阴影贴图 shader (只输出深度)
	directionalShadowShader.CreateFromFiles("Shaders/directional_shadow_map.vert", "Shaders/directional_shadow_map.frag");
	// 点光源 全向阴影贴图 (需要几何 Shader 扩展到 cubemap 的渲染)
	omniShadowShader.CreateFromFiles("Shaders/omni_shadow_map.vert", "Shaders/omni_shadow_map.geom", "Shaders/omni_shadow_map.frag");
}

/// <summary>
/// 把场景中所有物体绘制出来,用于主渲染与阴影渲染两种场景（不同 Shader 分别调用）
/// </summary>
void RenderScene()
{
	glm::mat4 model(1.0f);

	// 第一个物体(位置, 纹理, 材质, 渲染)
#pragma region objA
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	brickTexture.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();
#pragma endregion

	// 第二个物体
#pragma region objB
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 4.0f, -2.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dirtTexture.UseTexture();
	dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[1]->RenderMesh();
#pragma endregion

	// 地面
#pragma region floor
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dirtTexture.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[2]->RenderMesh();
#pragma endregion

	// 加载并渲染 x-wing 模型(model)
#pragma region X-Wing
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-7.0f, 0.0f, 10.0f));
	model = glm::scale(model, glm::vec3(0.006f, 0.006f, 0.006f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	xwing.RenderModel();
#pragma endregion

	// 加载并渲染 黑鹰直升机
#pragma region Blackhwak
	// 设置直升机 环绕 Y轴旋转
	blackhawkAngle += 0.1f;
	if (blackhawkAngle > 360.0f)
	{
		blackhawkAngle = 0.1f;
	}

	// 加载并渲染
	model = glm::mat4(1.0f);
	model = glm::rotate(model, -blackhawkAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-8.0f, 2.0f, 0.0f));
	model = glm::rotate(model, -20.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, -90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	blackhawk.RenderModel();
#pragma endregion
}

/// <summary>
/// 方向/平行光阴影贴图的渲染通道(只写入深度)
/// 1. 绑定方向光专用 shader
/// 2. 调整视口到 shadow map 大小
/// 3. 将 shadow FBO 绑定为绘制目标并清空深度
/// 4. 设置 light-space 矩阵（uniform）
/// 5. 渲染场景（只会写入深度）
/// </summary>
/// <param name="light">渲染的平行光对象指针</param>
void DirectionalShadowMapPass(DirectionalLight* light)
{
	directionalShadowShader.UseShader();

	glViewport(0, 0, light->getShadowMap()->GetShadowWidth(), light->getShadowMap()->GetShadowHeight());

	light->getShadowMap()->Write();			// 将渲染目标切换为 阴影贴图的 FBO
	glClear(GL_DEPTH_BUFFER_BIT);

	uniformModel = directionalShadowShader.GetModelLocation();
	auto lTransform = light->CalculateLightTransform();
	directionalShadowShader.SetDirectionalLightTransform(&lTransform);

	directionalShadowShader.Validate();

	RenderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/// <summary>
/// 为点光或聚光生成全向阴影贴图（写入立方体深度贴图）。
/// 需要把光源位置与 farPlane 等传给 shader，并提供 6 个 light matrices。
/// </summary>
void OmniShadowMapPass(PointLight* light)
{
	omniShadowShader.UseShader();

	glViewport(0, 0, light->getShadowMap()->GetShadowWidth(), light->getShadowMap()->GetShadowHeight());

	light->getShadowMap()->Write();
	glClear(GL_DEPTH_BUFFER_BIT);

	// 获取需要在 omni shadow shader 中写入的 uniform 位置信息
	uniformModel = omniShadowShader.GetModelLocation();
	uniformOmniLightPos = omniShadowShader.GetOmniLightPosLocation();
	uniformFarPlane = omniShadowShader.GetFarPlaneLocation();

	glUniform3f(uniformOmniLightPos, light->GetPosition().x, light->GetPosition().y, light->GetPosition().z);
	glUniform1f(uniformFarPlane, light->GetFarPlane());
	//  把 6 个投影视图矩阵传入 shader，每个对应切片 face
	omniShadowShader.SetLightMatrices(light->CalculateLightTransform());

	omniShadowShader.Validate();

	RenderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/// <summary>
/// 实际最终场景渲染:
/// 最终渲染过程：
/// 1. 设置视口回到窗口大小并清除屏幕
/// 2. 绘制天空盒（深度写入暂时关闭）
/// 3. 使用主 shader 绘制场景（设置各种 uniform 与绑定 shadow map）
/// </summary>
/// <param name="viewMatrix">摄像机的 view 矩阵</param>
/// <param name="projectionMatrix">投影矩阵</param>
void RenderPass(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
	glViewport(0, 0, 1920, 1080);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	skybox.DrawSkybox(viewMatrix, projectionMatrix);

	// 启用 主 shader
	shaderList[0].UseShader();

	// 获取并缓存 shader 中的 uniform位置以便设置
	uniformModel = shaderList[0].GetModelLocation();
	uniformProjection = shaderList[0].GetProjectionLocation();
	uniformView = shaderList[0].GetViewLocation();
	uniformModel = shaderList[0].GetModelLocation();
	uniformEyePosition = shaderList[0].GetEyePositionLocation();
	uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
	uniformShininess = shaderList[0].GetShininessLocation();

	// 上传投影/视图矩阵 与 观察者位置
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

	// 把光源数据写入 shader, 这里使用 textureUnit 3
	shaderList[0].SetDirectionalLight(&mainLight);
	shaderList[0].SetPointLights(pointLights, pointLightCount, 3, 0);
	shaderList[0].SetSpotLights(spotLights, spotLightCount, 3 + pointLightCount, pointLightCount);

	// 写入平行光的光照变换矩阵到 shader
	auto lTransform = mainLight.CalculateLightTransform();
	shaderList[0].SetDirectionalLightTransform(&lTransform);

	// 绑定方向光 shadow map 到 texture 2
	mainLight.getShadowMap()->Read(GL_TEXTURE2);
	shaderList[0].SetTexture(1);
	shaderList[0].SetDirectionalShadowMap(2);

	// 将摄像机稍微下移用于手电筒（spot light）位置，并同步方向
	glm::vec3 lowerLight = camera.getCameraPosition();
	lowerLight.y -= 0.3f;
	spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

	shaderList[0].Validate();

	// 渲染场景（所有物体），lighting + shadow 都在 shader 内计算
	RenderScene();
}
#pragma endregion


int main() 
{
	// 创建窗帘并初始化 OpenGL/GLFW/GLEW
	mainWindow = Window(1920, 1080); // 1280, 1024 or 1024, 768
	mainWindow.Initialise();

	// 创建 mesh 对象 和 shader
	CreateObjects();
	CreateShaders();
	
	// 摄像机初始化
	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 5.0f, 0.3f);

	// 加载纹理
	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();

	// 初始化 Material
	shinyMaterial = Material(4.0f, 256);
	dullMaterial = Material(0.3f, 4);

	// 加载 model
	xwing = Model();
	xwing.LoadModel("Models/x-wing.obj");

	blackhawk = Model();
	blackhawk.LoadModel("Models/uh60.obj");

	// 平行光
	mainLight = DirectionalLight(2048, 2048,
								1.0f, 0.53f, 0.3f, 
								0.1f, 0.9f,
								-10.0f, -12.0f, 18.5f);
	// 点光源
	pointLights[0] = PointLight(1024, 1024,
								0.01f, 100.0f,
								0.0f, 0.0f, 1.0f,
								0.0f, 1.0f,
								1.0f, 2.0f, 0.0f,
								0.3f, 0.2f, 0.1f);
	pointLightCount++;
	pointLights[1] = PointLight(1024, 1024,
								0.01f, 100.0f, 
								0.0f, 1.0f, 0.0f,
								0.0f, 1.0f,
								-4.0f, 3.0f, 0.0f,
								0.3f, 0.2f, 0.1f);
	pointLightCount++;

	// 聚光灯
	spotLights[0] = SpotLight(1024, 1024,
						0.01f, 100.0f, 
						1.0f, 1.0f, 1.0f,
						0.0f, 2.0f,
						0.0f, 0.0f, 0.0f,
						0.0f, -1.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						20.0f);
	spotLightCount++;
	spotLights[1] = SpotLight(1024, 1024,
						0.01f, 100.0f, 
						1.0f, 1.0f, 1.0f,
						0.0f, 1.0f,
						0.0f, -1.5f, 0.0f,
						-100.0f, -1.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						20.0f);
	spotLightCount++;

	// 天空盒
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	// 局部 uniform
	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	// 设置投影矩阵
	glm::mat4 projection = glm::perspective(glm::radians(60.0f),
		static_cast<float>(mainWindow.getBufferWidth() )/static_cast<float>( mainWindow.getBufferHeight()),
		0.1f, 100.0f);

	// Loop until window closed
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = static_cast<float>(glfwGetTime()); // SDL_GetPerformanceCounter();
		deltaTime = now - lastTime; // (now - lastTime)*1000/SDL_GetPerformanceFrequency();
		lastTime = now;

		// Get + Handle User Input
		glfwPollEvents();

		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// 切换手电筒（按 F 键）
		if (mainWindow.getsKeys()[GLFW_KEY_F])
		{
			spotLights[0].Toggle();
			mainWindow.getsKeys()[GLFW_KEY_F] = false;
		}

		// ===================== 生成阴影贴图 ============================
		DirectionalShadowMapPass(&mainLight);

		for (size_t i = 0; i < pointLightCount; i++)
		{
			OmniShadowMapPass(&pointLights[i]);
		}

		for (size_t i = 0; i < spotLightCount; i++)
		{
			OmniShadowMapPass(&spotLights[i]);
		}

		// ========================== 最终渲染阶段 ===========================
		RenderPass(camera.calculateViewMatrix(), projection);

		// 交换缓冲，显示
		mainWindow.swapBuffers();
	}

	return 0;
}