#ifndef LIGHT_H
#define LIGHT_H

#include "Components/Component.h"

enum LightType {GE_POINT_LIGHT, GE_SPOT_LIGHT, GE_DIRECTIONAL_LIGHT};

class Light : public Component
{
    public:
        Light(LightType _type = GE_POINT_LIGHT, vec3 _offset = vec3(0.0f), vec3 _diffuse = vec3(1.0f), float _ambient = 0.2f, float _constant = 1.0f, float _linear = 0.0f, float _quadratic = 0.005f);
        virtual ~Light();

        /// Methods (public)
            virtual Component* clone() override;

            virtual void registerComponent() override;
            virtual void deregisterComponent() override;

        /// Getters
            vec3 getPosition() const;

            vec3 getDiffuseColor() const;
            float getAmbientCoefficient() const;

            vec3 getAttenuation() const;

        /// Attributes (public)
            LightType type;

            vec3 position;

            vec3 diffuse;
            float ambient;

            vec3 attenuation;
};

#endif // LIGHT_H
