#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Components/Component.h"

class Transform : public Component
{
    public:
        Transform(vec3 _position, quat _rotation, vec3 _scale);
        Transform(vec3 _position = vec3(0.0f), vec3 _rotation = vec3(0.0f), vec3 _scale = vec3(1.0f));

        virtual ~Transform();

        /// Methods (public)
            virtual Component* clone() override;

            virtual void attach(Entity* _entity) override;
            virtual void detach() override;

            void toMatrix();
            void use() const;

            void lookAt(vec3 _target);

            bool isChildOf(Transform* _tr) const;

        /// Setters
            void setParent(Transform* _parent);

        /// Getters
            Transform* getParent() const;
            Transform* getRoot() const;
            std::list<Transform*> getChildren() const;

            vec3 getPosition() const;
            vec3 getDirection() const;

            vec3 getToLocalSpace(vec3 _point) const;
            mat4 getToLocalSpace(const mat4& _matrix) const;
            vec3 getVectorToLocalSpace(vec3 _vector) const;

            vec3 getToWorldSpace(vec3 _point) const;
            mat3 getToWorldSpace(const mat3& _matrix) const;
            mat4 getToWorldSpace(const mat4& _matrix) const;
            vec3 getVectorToWorldSpace(vec3 _vector) const;

        /// Attributes (public)
            mat4 toWorldSpace;
            mat4 toLocalSpace;

            vec3 position;
            quat rotation;
            vec3 scale;

    private:
        void setRoot(Transform* _root);

        /// Attributes (private)
            Transform *root, *parent;
            std::list<Transform*> children;
};

#endif // TRANSFORM_H
