#ifndef LIGHT_H
#define LIGHT_H

#include "Components/Component.h"

enum LightType {GE_POINT_LIGHT, GE_SPOT_LIGHT, GE_DIRECTIONAL_LIGHT};

class Light : public Component
{
	friend class Entity;

	public:
		Light(LightType _type = GE_POINT_LIGHT, vec3 _offset = vec3(0.0f),
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
			LightType type;

			vec3 offset, color;
};

#endif // LIGHT_H
