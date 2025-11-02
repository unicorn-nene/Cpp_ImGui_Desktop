#include "ShadowMap.h"

/// <summary>
/// 默认构造函数
/// </summary>
ShadowMap::ShadowMap()
	:FBO_(0)
	,shadowMap_(0)
{

}

/// <summary>
/// 初始化阴影贴图(shadow map) ,帧缓冲(FBO) 和 深度纹理
/// </summary>
/// <param name="width">阴影贴图的宽度。</param>
/// <param name="height">阴影贴图的高度。</param>
/// <returns>若帧缓冲创建成功返回 true，创建失败返回 false</returns>
bool ShadowMap::Init(unsigned int width, unsigned int height)
{
	shadowWidth_ = width;
	shadowHeight_ = height;

	// 生成帧缓冲对象
	glGenFramebuffers(1, &FBO_);

	// 创建并绑定深度纹理
	glGenTextures(1, &shadowMap_);
	glBindTexture(GL_TEXTURE_2D, shadowMap_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth_, shadowHeight_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	// 设置纹理过滤 线性插值
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 设置纹理环绕方式 边界裁剪
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// 设定边界颜色为白色, 用于超出范围的采样
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// 将 纹理附加到 FBO的深度附件(Depth attachment)
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO_);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap_, 0);

	// 禁止颜色输出, 只渲染深度
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
void ShadowMap::Write()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO_);
}

/// <summary>
/// 在渲染阴影时，将阴影贴图绑定到指定的纹理单元（Texture Unit). 供后续的片元着色器采样。
/// 调用 Read() -> Shader 采样深度, 比较片元深度与光源深度判断是否在阴影中
/// </summary>
/// <param name="TextureUnit">要绑定的纹理单元</param>
void ShadowMap::Read(GLenum texUnit)
{
	glActiveTexture(texUnit);
	glBindTexture(GL_TEXTURE_2D, shadowMap_);
}

/// <summary>
/// 析构函数
/// </summary>
ShadowMap::~ShadowMap()
{
	if (FBO_)
	{
		glDeleteFramebuffers(1, &FBO_);
	}

	if (shadowMap_)
	{
		glDeleteTextures(1, &shadowMap_);
	}
}
