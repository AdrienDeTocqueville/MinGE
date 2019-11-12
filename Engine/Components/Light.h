#pragma once

#include "Components/Component.h"

class Light : public Component
{
	friend class Entity;
public:
	enum Type
	{
		Point,
		Spot,
		Directional,
	};

	Light(Light::Type _type, vec3 _offset = vec3(0.0f),
			vec3 _color = vec3(150.0f / 255.0f));
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

	/// Attributes (private)
	Light::Type type;

	vec3 offset, color;
};
