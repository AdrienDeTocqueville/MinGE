#pragma once

#include <memory>

#include "Assets/Texture.h"

typedef std::shared_ptr<class RenderTarget> RenderTargetRef;

class RenderTarget
{
	friend class Light;
	friend class Camera;

public:
	enum DepthType
	{
		DEPTH_16_BIT = GL_DEPTH_COMPONENT16,
		DEPTH_24_BIT = GL_DEPTH_COMPONENT24,
		DEPTH_32_BIT = GL_DEPTH_COMPONENT32,
		NONE = 0
	};

	static RenderTargetRef create(uvec2 size, DepthType depth = DEPTH_16_BIT);
	static RenderTargetRef getDefault();
	~RenderTarget();

	void resize(uvec2 _size);

	const Texture* getColorBuffer() const;
	const RenderBuffer* getDepthBuffer() const;

	vec2 getSize() const;

private:
	RenderTarget(uvec2 _size);
	RenderTarget(uvec2 _size, unsigned _fbo, Texture &&_color, RenderBuffer &&_depth);

	unsigned fbo;
	uvec2 size;

	Texture colorBuffer;
	RenderBuffer depthBuffer;

	static std::weak_ptr<RenderTarget> basic;
};
