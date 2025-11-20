#pragma once

#include <stdio.h>

#include <GL\glew.h>

/// <summary>
/// ShaderMap(阴影贴图)类: 创建一个阴影贴图(ShadowMap), 实现阴影映射(Shadow Mapping)
/// </summary>
class ShadowMap
{
public:
	ShadowMap();

	virtual bool Init(unsigned int width, unsigned int height);

	virtual void Write();
	virtual void Read(GLenum TextureUnit);

	GLuint GetShadowWidth() { return shadowWidth_; }
	GLuint GetShadowHeight() { return shadowHeight_; }

	~ShadowMap();

protected:
	GLuint FBO_{};					 ///< 帧缓冲对象(FrameBuffer object)ID
	GLuint shadowMap_{};			 ///< 阴影贴图纹理(Depth Texture)ID

	GLuint shadowWidth_{};			 ///< 阴影贴图宽度
	GLuint shadowHeight_{};			 ///< 阴影贴图高度
};