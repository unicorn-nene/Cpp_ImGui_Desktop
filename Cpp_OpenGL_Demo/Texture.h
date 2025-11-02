#pragma once

#include <GL\glew.h>

#include "CommonValues.h"

/// <summary>
/// 封装 OpenGL Texture(纹理)对象的加载, 使用与清理.
/// 支持普通 Texture 和 透明通道的 Texture 加载
/// </summary>
class Texture
{
public:
	Texture();
	Texture(const char* fileLoc);

	bool LoadTexture();
	bool LoadTextureA();

	void UseTexture();
	void ClearTexture();

	~Texture();

private:
	GLuint textureID_{};				///< OpenGL 纹理对象 ID
	int width_;							///< texture(纹理) 像素宽度
	int height_;						///< texture(纹理) 像素高度
	int bitDepth_;						///< texture(纹理) 像素深度

	const char* fileLocation_{};		///< texture(纹理) 文件路径
};

