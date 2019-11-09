#pragma once

#include <memory>

#include "Renderer/CommandBucket.h"
#include "Assets/Texture.h"

typedef std::shared_ptr<class RenderTarget> RenderTargetRef;

class RenderTarget
{
	friend class Camera;

	enum Depth
	{
		DEPTH_16_BIT = GL_DEPTH_COMPONENT16,
		DEPTH_24_BIT = GL_DEPTH_COMPONENT24,
		NONE = 0
	};

public:
	static RenderTargetRef create(uvec2 size, Depth depth = DEPTH_16_BIT, unsigned priority = 0);
	static RenderTargetRef getDefault();
	~RenderTarget();

	void bind() const;
	void resize(uvec2 _size);

	const Texture* getColorBuffer() const;
	const RenderBuffer* getDepthBuffer() const;

	vec2 getSize() const;
	unsigned getPriority() const;

	CommandBucket bucket;

private:
	RenderTarget(uvec2 _size);
	RenderTarget(uvec2 _size, unsigned _fbo, Texture &&_color, RenderBuffer &&_depth, unsigned priority);

	unsigned fbo, priority;
	uvec2 size;

	Texture colorBuffer;
	RenderBuffer depthBuffer;

	static std::weak_ptr<RenderTarget> basic;
};
