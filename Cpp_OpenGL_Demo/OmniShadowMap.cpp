#include "OmniShadowMap.h"

/// <summary>
/// 默认构造函数
/// </summary>
OmniShadowMap::OmniShadowMap() 
	: ShadowMap()
{
}

/// <summary>
/// 初始化 立方体贴图
/// </summary>
/// <param name="width">阴影贴图宽度</param>
/// <param name="height">阴影贴图高度</param>
/// <returns>初始化成功返回 true, 初始化失败返回 false</returns>
bool OmniShadowMap::Init(unsigned int width, unsigned int height)
{
	shadowWidth_ = width; shadowHeight_ = height;

	glGenFramebuffers(1, &FBO_);

	glGenTextures(1, &shadowMap_);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap_);

	// 为立方体的 6个面生成深度纹理
	for (size_t i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, shadowWidth_, shadowHeight_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	// 设置立方体贴图,线性过滤 + 边缘裁剪
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	// 将立方体贴图附加到 帧缓冲的 深度附件(depth attachment)
	glBindFramebuffer(GL_FRAMEBUFFER, FBO_);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMap_, 0);
	
	// 禁止颜色写入, 只渲染深度
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	
	// 检查帧缓冲是否完整
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Framebuffer error: %d\n", Status);
		return false;
	}

	return true;
}

/// <summary>
/// 绑定当前阴影贴图的帧缓冲对象（FBO）以进行深度渲染. 调用后，后续的绘制命令会写入该阴影贴图
/// 调用 Wirte() -> 渲染场景(光源视角),生成深度贴图
/// </summary>
void OmniShadowMap::Write()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO_);
}

/// <summary>
/// 在渲染阴影时，将阴影贴图绑定到指定的纹理单元（Texture Unit). 供后续的片元着色器采样。
/// 调用 Read() -> Shader 采样深度, 比较片元深度与光源深度判断是否在阴影中
/// </summary>
/// <param name="TextureUnit">要绑定的纹理单元</param>
void OmniShadowMap::Read(GLenum texUnit)
{
	glActiveTexture(texUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap_);
}

/// <summary>
/// 析构函数, 父类实现完成资源释放
/// </summary>
OmniShadowMap::~OmniShadowMap()
{
}
