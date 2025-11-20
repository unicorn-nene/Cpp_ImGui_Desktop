#pragma once

#include <GL\glew.h>

/// <summary>
/// 封装一个 Mesh(网格) 对象的创建,渲染和清理操作
/// 使用 VAO, VBO, IBO 方式向 GPU 上传 vertex(顶点)与 index(索引)数据.
/// </summary>
class Mesh
{
public:
	Mesh();

	void CreateMesh(GLfloat *vertices, unsigned int *indices, unsigned int numOfVertices, unsigned int numOfIndices);
	void RenderMesh();
	void ClearMesh();

	~Mesh();

private:
	GLuint VAO_{};			///< 顶点数组对象（Vertex Array Object）
	GLuint VBO_{};			///< 顶点缓冲对象（Vertex Buffer Object）
	GLuint IBO_{};			///< 索引缓冲对象（Index Buffer Object）

	GLsizei indexCount_{};	///< 索引数量，用于渲染调用
};

