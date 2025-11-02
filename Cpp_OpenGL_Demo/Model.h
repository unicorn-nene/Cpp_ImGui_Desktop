#pragma once

#include <vector>
#include <string>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include "Mesh.h"
#include "Texture.h"

/// <summary>
/// 封装了使用 Assimp 加载和渲染 3D 模型的功能。
/// Model 类负责从文件加载模型、提取网格（Mesh）与材质（Texture）数据，
/// 并在渲染时根据材质绑定相应纹理再绘制网格。
/// </summary>
class Model
{
public:
	Model();

	void LoadModel(const std::string& fileName);
	void RenderModel();
	void ClearModel();

	~Model();

private:
	void LoadNode(aiNode *node, const aiScene *scene);
	void LoadMesh(aiMesh *mesh, const aiScene *scene);
	void LoadMaterials(const aiScene *scene);

private:
	std::vector<Mesh*> meshList_{};				   ///< 存储 Model 中所有 Mesh的指针, 每个 Mesh 对应 Model 一个独立的部分
	std::vector<Texture*> textureList_{};		   ///< 存储与每个材质索引对应的 texture
	std::vector<unsigned int> meshToTex_{};		   ///< 将每个 Mesh 索引映射到其对应的 Material的索引
};

