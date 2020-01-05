#pragma once

#include "Components/Component.h"
#include "Assets/RenderTarget.h"

class Light : public Component
{
	friend class GraphicEngine;

public:
	enum Type
	{
		Point,
		Spot,
		Directional,
	};

	Light(Light::Type _type, vec3 _color = vec3(150.0f / 255.0f), bool _cast_shadow = false);
	virtual ~Light();

	/// Methods (public)
	virtual Light* clone() const override;

	void bind() const;

	/// Getters
	vec3 getPosition() const;
	vec3 getDirection() const;
	vec3 getColor() const { return color; }

	static Light *main;

private:
	/// Methods (private)
	virtual void onRegister() override;
	virtual void onDeregister() override;

	void update(struct View *view);

	/// Attributes (private)
	Light::Type type;
	vec3 color;

	bool cast_shadow;
	RenderTargetRef target;
	mat4 light_space;

	static Light *bound;
};
