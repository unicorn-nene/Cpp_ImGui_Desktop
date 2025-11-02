#include "Texture.h"

/// <summary>
/// 默认构造函数
/// </summary>
Texture::Texture()
	:textureID_(0)
	,width_(0)
	,height_(0)
	,bitDepth_(0)
	,fileLocation_("")
{

}

/// <summary>
/// 使用 Texture对象文件路径构造 Texture 对象
/// </summary>
/// <param name="fileLoc"></param>
Texture::Texture(const char* fileLoc)
	:textureID_(0)
	, width_(0)
	, height_(0)
	, bitDepth_(0)
	, fileLocation_(fileLoc)
{
	
}

/// <summary>
/// 加载三通道 texture文件(RGB格式), 自动生成 OpenGL Texture对象, 绑定并上传图像数据.
/// </summary>
/// <returns>加载成功返回 true, 加载失败返回 false</returns>
bool Texture::LoadTexture()
{
	unsigned char *texData = stbi_load(fileLocation_, &width_, &height_, &bitDepth_, 0);
	if (!texData)
	{
		printf("Failed to find: %s\n", fileLocation_);
		return false;
	}

	glGenTextures(1, &textureID_);
	glBindTexture(GL_TEXTURE_2D, textureID_);

	// 纹理环绕模式 - 重复纹理
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// 纹理过滤模式 - 线性过滤
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(texData);

	return true;
}

/// <summary>
/// 加载四通道(含 Alpha) texture文件(RGBA格式), 自动生成 OpenGL Texture对象, 绑定并上传图像数据.
/// </summary>
/// <returns>加载成功返回 true, 加载失败返回 false</returns>
bool Texture::LoadTextureA()
{
	unsigned char *texData = stbi_load(fileLocation_, &width_, &height_, &bitDepth_, 0);
	if (!texData)
	{
		printf("Failed to find: %s\n", fileLocation_);
		return false;
	}

	glGenTextures(1, &textureID_);
	glBindTexture(GL_TEXTURE_2D, textureID_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(texData);

	return true;
}

/// <summary>
/// 使用这个 Texture 对象
/// </summary>
void Texture::UseTexture()
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureID_);
}

/// <summary>
/// 清理释放 Texture 资源
/// </summary>
void Texture::ClearTexture()
{
	glDeleteTextures(1, &textureID_);
	textureID_ = 0;
	width_ = 0;
	height_ = 0;
	bitDepth_ = 0;
	fileLocation_ = "";
}

/// <summary>
/// 析构函数
/// </summary>
Texture::~Texture()
{
	ClearTexture();
}
