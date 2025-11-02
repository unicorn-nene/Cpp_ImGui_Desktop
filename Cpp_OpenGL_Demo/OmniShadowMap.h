#pragma once
#include "ShadowMap.h"

/// <summary>
/// OmniShadowMap 继承 ShadowMap 类,用于记录点光源的阴影贴图
/// OmniShadowMap 类 使用立方体贴图(cube Map),能记录从光源出发的 6个方向的深度信息
/// </summary>
class OmniShadowMap :
	public ShadowMap
{
public:
	OmniShadowMap();

	bool Init(unsigned int width, unsigned int height);

	void Write();

	void Read(GLenum TextureUnit);

	~OmniShadowMap();
};

