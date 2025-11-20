#include "Model.h"

/// <summary>
/// 默认构造函数
/// </summary>
Model::Model()
{
}

/// <summary>
/// 使用 Assimp 加载模型文件, 该函数会解析文件结构，自动生成网格和材质资源。
/// </summary>
/// <param name="fileName">model文件路径</param>
void Model::LoadModel(const std::string& fileName)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

	if (!scene)
	{
		printf("Model (%s) failed to load: %s", fileName.c_str(), importer.GetErrorString());
		return;
	}

	LoadNode(scene->mRootNode, scene);

	LoadMaterials(scene);
}

/// <summary>
/// 渲染模型中所有的网格部分。
/// 根据每个网格的材质索引绑定对应纹理后调用 Mesh::RenderMesh() 绘制。
/// </summary>
void Model::RenderModel()
{
	for (size_t i = 0; i < meshList_.size(); i++)
	{
		unsigned int materialIndex = meshToTex_[i];

		if (materialIndex < textureList_.size() && textureList_[materialIndex])
		{
			textureList_[materialIndex]->UseTexture();
		}

		meshList_[i]->RenderMesh();
	}
}


/// <summary>
/// 清理资源
/// </summary>
void Model::ClearModel()
{
	for (size_t i = 0; i < meshList_.size(); i++)
	{
		if (meshList_[i])
		{
			delete meshList_[i];
			meshList_[i] = nullptr;
		}
	}

	for (size_t i = 0; i < textureList_.size(); i++)
	{
		if (textureList_[i])
		{
			delete textureList_[i];
			textureList_[i] = nullptr;
		}
	}
}

/// <summary>
/// 析构函数
/// </summary>
Model::~Model()
{
	ClearModel();
}


///============================= private function =====================================

/// <summary>
/// 递归加载 aiNode的节点及其子节点
/// </summary>
/// <param name="node">要加载的 aiNode指针</param>
/// <param name="scene">包含 Node的 aiScene指针</param>
void Model::LoadNode(aiNode* node, const aiScene* scene)
{
	// 加载当前节点的所有网络
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}

	// 递归加载所有子节点
	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}
}

/// <summary>
/// 从 aiMesh 对象中提取顶点、法线、纹理坐标与索引数据，并创建对应的 Mesh 实例
/// </summary>
/// <param name="mesh">要加载的 aiMesh 指针</param>
/// <param name="scene">包含 Mesh的 aiScene 对象指针</param>
void Model::LoadMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;

	// 处理顶点属性
	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		// 顶点 (x, y, z)
		vertices.insert(vertices.end(), { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });
		// 纹理坐标(u, v)
		if (mesh->mTextureCoords[0])
		{
			vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
		}
		else {
			vertices.insert(vertices.end(), { 0.0f, 0.0f });
		}
		//法线向量 (nx, ny, nz)
		vertices.insert(vertices.end(), { -mesh->mNormals[i].x, -mesh->mNormals[i].y, -mesh->mNormals[i].z });
	}

	// 加载索引
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// 创建新的 Mesh 实例
	Mesh* newMesh = new Mesh();
	newMesh->CreateMesh(&vertices[0], &indices[0], vertices.size(), indices.size());
	meshList_.push_back(newMesh);
	meshToTex_.push_back(mesh->mMaterialIndex);
}

/// <summary>
/// 加载 Model的所有材质(material)
/// 若模型未定义纹理，则自动使用默认的 plain.png。
/// </summary>
/// <param name="scene">Assimp 导入的 aiScene 对象</param>
void Model::LoadMaterials(const aiScene* scene)
{
	textureList_.resize(scene->mNumMaterials);

	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];

		textureList_[i] = nullptr;

		// 如果 material 存在, 构造漫反射贴图(diffuse)
		if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				// 提取文件名
				int idx = std::string(path.data).rfind("\\");
				std::string filename = std::string(path.data).substr(idx + 1);
				// 构造纹理路径
				std::string texPath = std::string("Textures/") + filename;

				textureList_[i] = new Texture(texPath.c_str());
				if (!textureList_[i]->LoadTexture())
				{
					printf("Failed to load texture at: %s\n", texPath.c_str());
					delete textureList_[i];
					textureList_[i] = nullptr;
				}
			}
		}

		// 若材质无纹理, 则使用默认纹理
		if (!textureList_[i])
		{
			textureList_[i] = new Texture("Textures/plain.png");
			textureList_[i]->LoadTextureA();
		}
	}
}
