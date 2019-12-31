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

	Light(Light::Type _type, vec3 _color = vec3(150.0f / 255.0f), bool _cast_shadows = false);
	virtual ~Light();

	/// Methods (public)
	virtual Light* clone() const override;

	/// Getters
	vec3 getPosition() const;
	vec3 getColor() const;

private:
	/// Methods (private)
	virtual void onRegister() override;
	virtual void onDeregister() override;

	void update(struct View *view);

	/// Attributes (private)
	Light::Type type;
	vec3 color;
	RenderTargetRef target;

	static Light *main;
};
