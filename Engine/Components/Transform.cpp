#include "Components/Transform.h"

Transform::Transform(vec3 _position, quat _rotation, vec3 _scale):
	position(_position), rotation(_rotation), scale(_scale),
	root(this), parent(nullptr),
	validWorld(false), validLocal(false)
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
	validWorld = validLocal = false;

	for (Transform *child : children)
		child->updateChildren();
}

void Transform::lookAt(vec3 _target)
{
	rotation = glm::rotation(vec3(1, 0, 0), normalize(_target - position));
	toMatrix();
}

bool Transform::isChildOf(Transform* _tr) const
{
	return (parent == _tr);
}

/// Setters
void Transform::setParent(Transform* _parent)
{
	if (parent != nullptr)
	{
		auto it = std::find(parent->children.begin(), parent->children.end(), this);
		*it = parent->children.back();
		parent->children.pop_back();
	}

	parent = _parent;

	if (_parent != nullptr)
	{
		parent->children.push_back(this);
		setRoot(parent->root);
	}
	else
		setRoot(this);
}

void Transform::setPosition(vec3 _position)
{
	position = _position;
	toMatrix();
}

void Transform::setRotation(vec3 _rotation)
{
	rotation = quat(_rotation);
	toMatrix();
}

void Transform::setScale(vec3 _scale)
{
	scale = _scale;
	toMatrix();
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

std::vector<Transform*> Transform::getChildren() const
{
	return children;
}

vec3 Transform::getPosition()
{
	if (parent)
		return toWorld(vec3(0.0f));
	else
		return position;
}

vec3 Transform::getDirection()
{
	return vectorToWorld(vec3(1, 0, 0));
}

const mat4 &Transform::getToWorld()
{
	computeWorldMatrix();
	return world;
}

const mat4 &Transform::getToLocal()
{
	computeLocalMatrix();
	return local;
}

vec3 Transform::toLocal(vec3 _point)
{
	computeLocalMatrix();

	return vec3(local * vec4(_point, 1.0f));
}

mat4 Transform::toLocal(const mat4& _matrix)
{
	computeLocalMatrix();

	return local * _matrix;
}

vec3 Transform::vectorToLocal(vec3 _vector)
{
	return toLocal(_vector + position);
}

vec3 Transform::toWorld(vec3 _point)
{
	computeWorldMatrix();

	return vec3(world * vec4(_point, 1.0f));
}

mat3 Transform::toWorld(const mat3& _matrix)
{
	mat3 m = toMat3(rotation);
	return m * _matrix * transpose(m);
}

mat4 Transform::toWorld(const mat4& _matrix)
{
	computeWorldMatrix();

	return world * _matrix;
}

vec3 Transform::vectorToWorld(vec3 _vector)
{
	return toWorld(_vector) - position;
}

/// Methods (private)
void Transform::setRoot(Transform* _root)
{
	validWorld = validLocal = false;

	root = _root;
	for (Transform* child: children)
		child->setRoot(root);
}

void Transform::updateChildren()
{
	if (!validWorld) // means validLocal if false too
		return; // already dirty, no need to tell children

	validWorld = validLocal = false;
	for (Transform *child : children)
		child->updateChildren();
}

void Transform::computeWorldMatrix()
{
	if (!validWorld)
	{
		if (parent != nullptr)
			simd_mul(parent->getToWorld(), glm::translate(position), world);
		else
			world = glm::translate(position);

		simd_mul(world, toMat4(rotation), world);
		simd_mul(world, glm::scale(scale), world);

		validWorld = true;
		validLocal = false;
	}
}

void Transform::computeLocalMatrix()
{
	computeWorldMatrix();

	if (!validLocal)
	{
		local = inverse(world);
		validLocal = true;
	}
}
