#pragma once

#include "Components/Component.h"
#include "Assets/RenderTarget.h"

class Entity;
class Transform;

class Camera : public Component
{
	friend class GraphicEngine;

public:
	Camera(float _FOV, float _zNear, float _zFar, vec3 _clearColor = vec3(0.0f), RenderTargetRef _target = nullptr,
		bool _orthographic = false, vec4 _viewport = vec4(0.0f,0.0f,1.0f,1.0f),
		unsigned _clearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
		unsigned _priority = 0);

	/// Methods (public)
	virtual Camera* clone() const override;

	/// Getters
	RenderTargetRef getRenderTarget() const;

	vec3 getPosition() const;
	vec3 getDirection() const;

	float getFOV() const;
	float getAspectRatio() const;
	unsigned getPriority() const { return priority; }

private:
	/// Methods (private)
	virtual void onRegister() override;
	virtual void onDeregister() override;

	void update(struct View *view);
	void computeViewPort();

	/// Attributes
	float FOV, zNear, zFar;
	unsigned priority;

	vec3 clearColor;
	unsigned clearFlags;

	bool orthographic;

	ivec4 viewport;
	vec4 relViewport;	// in screen coordinates (between 0 and 1)

	mat4 projection;

	RenderTargetRef renderTarget;
};
