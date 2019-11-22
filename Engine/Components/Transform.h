#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Components/Component.h"

class Transform : public Component
{
	friend class Entity;

	public:
		Transform(vec3 _position, quat _rotation, vec3 _scale);
		Transform(vec3 _position = vec3(0.0f), vec3 _rotation = vec3(0.0f), vec3 _scale = vec3(1.0f));

		virtual ~Transform();

		/// Methods (public)
			virtual Transform* clone() const override;

			void toMatrix();

			void lookAt(vec3 _target);

			bool isChildOf(Transform* _tr) const;

		/// Setters
			void setParent(Transform* _parent);

			void setPosition(vec3 _position);
			void setRotation(vec3 _rotation);
			void setScale(vec3 _scale);

			void setDirection(vec3 _direction);

		/// Getters
			Transform* getParent() const;
			Transform* getRoot() const;
			std::vector<Transform*> getChildren() const;

			vec3 getPosition();
			vec3 getDirection();

			const mat4 &getToWorld();
			const mat4 &getToLocal();

		// Transform helpers
			vec3 toLocal(vec3 _point);
			mat4 toLocal(const mat4& _matrix);
			vec3 vectorToLocal(vec3 _vector);

			vec3 toWorld(vec3 _point);
			mat3 toWorld(const mat3& _matrix);
			mat4 toWorld(const mat4& _matrix);
			vec3 vectorToWorld(vec3 _vector);

		// TODO: make them private
		/// Attributes (public)
			vec3 position;
			quat rotation;
			vec3 scale;

	private:
		/// Methods (private)
			void setRoot(Transform* _root);
			void updateChildren();

			void computeWorldMatrix();
			void computeLocalMatrix();

		/// Attributes (private)
			Transform *root, *parent;
			std::vector<Transform*> children;

			mat4 world, local;
			bool validWorld, validLocal;
};

#endif // TRANSFORM_H
