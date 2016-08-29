#include "Components/Transform.h"

#include "Systems/GraphicEngine.h"

Transform::Transform(vec3 _position, quat _rotation, vec3 _scale):
    position(_position), rotation(_rotation), scale(_scale),
    root(this), parent(nullptr)
{
    toMatrix();
}

Transform::Transform(vec3 _position, vec3 _rotation, vec3 _scale):
    Transform(_position, quat(_rotation), _scale)
{ }

Transform::~Transform()
{ }

/// Methods (public)
Component* Transform::clone()
{
    return new Transform(position, rotation, scale);
}

void Transform::attach(Entity* _entity)
{
    if (entity != nullptr)  // If already attached
        return;             // Refuse to change

    if (_entity->tr != nullptr)
        return;

    entity = _entity;       // Register entity
    entity->tr = this;      // Set the shortcut

    entity->components[typeid(Transform).hash_code()].push_back(this);   // Register component
}

void Transform::detach()
{
    return; // Impossible to detach transform
}

void Transform::toMatrix()
{
    if (parent != nullptr)
        toWorldSpace = parent->toWorldSpace;
    else
        toWorldSpace = mat4(1.0f);

    toWorldSpace *= glm::translate(position);
    toWorldSpace *= toMat4(rotation);
    toWorldSpace *= glm::scale(scale);

    toLocalSpace = inverse(toWorldSpace);

    for (Transform* child: children)
        child->toMatrix();
}

void Transform::use() const
{
    GraphicEngine::get()->setMatrix(GE_MODEL, toWorldSpace);

    GraphicEngine::get()->computeMVP();
}

void Transform::lookAt(vec3 _target)
{
    rotation = glm::rotation(vec3(1, 0, 0), normalize(_target - position));
}

bool Transform::isChildOf(Transform* _tr) const
{
    return (parent == _tr);
}

/// Setters
void Transform::setParent(Transform* _parent)
{
    if (parent != nullptr)
        parent->children.remove(this);

    parent = _parent;

    if (_parent != nullptr)
    {
        parent->children.push_back(this);
        root = parent->root;
    }
    else
        root = this;

    for (Transform* child: children)
        child->setRoot(root);
}

void Transform::setRoot(Transform* _root) // private
{
    root = _root;
}

/// Getters
Transform* Transform::getParent() const
{
    return parent;
}

Transform* Transform::getRoot() const
{
    return root;
}

std::list<Transform*> Transform::getChildren() const
{
    return children;
}

vec3 Transform::getPosition() const
{
    return position;
}

vec3 Transform::getDirection() const
{
    return getVectorToWorldSpace(vec3(1, 0, 0));
}

vec3 Transform::getToLocalSpace(vec3 _point) const
{
    return vec3(toLocalSpace * vec4(_point, 1.0f));
}

mat4 Transform::getToLocalSpace(const mat4& _matrix) const
{
    return toLocalSpace * _matrix;
}

vec3 Transform::getVectorToLocalSpace(vec3 _vector) const
{
    return vec3(toLocalSpace * vec4(_vector + position, 1.0f));
}

vec3 Transform::getToWorldSpace(vec3 _point) const
{
    return vec3(toWorldSpace * vec4(_point, 1.0f));
}

mat3 Transform::getToWorldSpace(const mat3& _matrix) const
{
    mat3 m = toMat3(rotation);
    return m * _matrix * transpose(m);
}

mat4 Transform::getToWorldSpace(const mat4& _matrix) const
{
    return toWorldSpace * _matrix;
}

vec3 Transform::getVectorToWorldSpace(vec3 _vector) const
{
    return vec3(toWorldSpace * vec4(_vector, 1.0f)) - position;
}
