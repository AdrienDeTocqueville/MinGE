#include "Components/Transform.h"

#include "Systems/GraphicEngine.h"

#include <glm/gtx/transform.hpp>

Transform::Transform(vec3 _position, quat _rotation, vec3 _scale):
    validWorld(false), validLocal(false),
    position(_position), rotation(_rotation), scale(_scale),
    root(this), parent(nullptr)
{ }

Transform::Transform(vec3 _position, vec3 _rotation, vec3 _scale):
    Transform(_position, quat(_rotation), _scale)
{ }

Transform::~Transform()
{ }

/// Methods (public)
Transform* Transform::clone() const
{
    return new Transform(position, rotation, scale);
}

void Transform::toMatrix()
{
    validWorld = validLocal = true;

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
    computeWorldSpaceMatrix();

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

void Transform::setPosition(vec3 _position)
{
    position = _position;

    validLocal = validWorld = false;
}

void Transform::setRotation(vec3 _rotation)
{
    rotation = quat(_rotation);

    validLocal = validWorld = false;
}

void Transform::setScale(vec3 _scale)
{
    scale = _scale;

    validLocal = validWorld = false;
}

void Transform::setDirection(vec3 _direction)
{
    rotation = glm::rotation(vec3(1, 0, 0), _direction);

    validLocal = validWorld = false;
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
    computeLocalSpaceMatrix();

    return vec3(toLocalSpace * vec4(_point, 1.0f));
}

mat4 Transform::getToLocalSpace(const mat4& _matrix) const
{
    computeLocalSpaceMatrix();

    return toLocalSpace * _matrix;
}

vec3 Transform::getVectorToLocalSpace(vec3 _vector) const
{
    return getToLocalSpace(_vector + position);
}

vec3 Transform::getToWorldSpace(vec3 _point) const
{
    computeWorldSpaceMatrix();

    return vec3(toWorldSpace * vec4(_point, 1.0f));
}

mat3 Transform::getToWorldSpace(const mat3& _matrix) const
{
    mat3 m = toMat3(rotation);
    return m * _matrix * transpose(m);
}

mat4 Transform::getToWorldSpace(const mat4& _matrix) const
{
    computeWorldSpaceMatrix();

    return toWorldSpace * _matrix;
}

vec3 Transform::getVectorToWorldSpace(vec3 _vector) const
{
    return getToWorldSpace(_vector) - position;
}

/// Methods (private)
void Transform::setRoot(Transform* _root) // private
{
    root = _root;
}

void Transform::computeWorldSpaceMatrix() const
{
    if (!validWorld)
    {
        toWorldSpace = glm::translate(position);
        toWorldSpace *= toMat4(rotation);
        toWorldSpace *= glm::scale(scale);

        validWorld = true;
        validLocal = false;
    }
}

void Transform::computeLocalSpaceMatrix() const
{
    computeWorldSpaceMatrix();

    if (!validLocal)
    {
        toLocalSpace = inverse(toWorldSpace);

        validLocal = true;
    }
}
