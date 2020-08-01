#pragma once

/// Transform
void Transform::set_position(vec3 pos)
{
	sys.data.get<0>(id())->position = pos;
	sys.update_matrices(id());
}

void Transform::set_rotation(vec3 rot)
{
	set_rotation(quat(rot));
}

void Transform::set_rotation(quat rot)
{
	sys.data.get<0>(id())->rotation = rot;
	sys.update_matrices(id());
}

void Transform::set_scale(vec3 scale)
{
	sys.data.get<0>(id())->scale = scale;
	sys.update_matrices(id());
}


void Transform::translate(vec3 vec)
{
	sys.data.get<0>(id())->position += vec;
	sys.update_matrices(id());
}

void Transform::rotate(vec3 axis, float angle)
{
	sys.data.get<0>(id())->rotation *= glm::angleAxis(angle, axis);
	sys.update_matrices(id());
}

void Transform::look_at(vec3 point)
{
	TransformSystem::transform_t *tr = sys.data.get<0>(id());

	tr->rotation = glm::rotation(vec3(1, 0, 0), normalize(point - tr->position));
	sys.update_matrices(id());
}


vec3 Transform::position() const
{ return sys.data.get<0>(id())->position; }

vec3 Transform::euler_angles() const
{ return eulerAngles(sys.data.get<0>(id())->rotation); }

quat Transform::rotation() const
{ return sys.data.get<0>(id())->rotation; }

vec3 Transform::scale() const
{ return sys.data.get<0>(id())->scale; }


const mat4& Transform::world_matrix() const
{ return sys.data.get<0>(id())->world; }

const mat4& Transform::local_matrix() const
{ return sys.data.get<0>(id())->local; }

mat4 Transform::normal_matrix() const
{ return transpose(inverse(world_matrix())); }


vec3 Transform::to_world(vec3 point) const
{
	TransformSystem::transform_t *tr = sys.data.get<0>(id());
	return vec3(tr->world * vec4(point, 1.0f));
}

vec3 Transform::vec_to_world(vec3 vec) const
{
	TransformSystem::transform_t *tr = sys.data.get<0>(id());
	return mat3(tr->world) * vec;
}
