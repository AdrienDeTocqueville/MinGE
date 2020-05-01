#pragma once

/// Transform
void Transform::set_position(vec3 pos)
{
	sys.data.get<0>(index)->position = pos;
	sys.update_matrices(index);
}

void Transform::set_rotation(vec3 rot)
{
	sys.data.get<0>(index)->rotation = quat(rot);
	sys.update_matrices(index);
}

void Transform::set_rotation(quat rot)
{
	sys.data.get<0>(index)->rotation = rot;
	sys.update_matrices(index);
}

void Transform::set_scale(vec3 scale)
{
	sys.data.get<0>(index)->scale = scale;
	sys.update_matrices(index);
}


void Transform::translate(vec3 vec)
{
	sys.data.get<0>(index)->position += vec;
	sys.update_matrices(index);
}

void Transform::look_at(vec3 point)
{
	TransformSystem::transform_t *tr = sys.data.get<0>(index);

	tr->rotation = glm::rotation(vec3(1, 0, 0), normalize(point - tr->position));
	sys.update_matrices(index);
}


vec3 Transform::position() const
{ return sys.data.get<0>(index)->position; }

vec3 Transform::euler_angles() const
{ return eulerAngles(sys.data.get<0>(index)->rotation); }

quat Transform::rotation() const
{ return sys.data.get<0>(index)->rotation; }

vec3 Transform::scale() const
{ return sys.data.get<0>(index)->scale; }


const mat4& Transform::world_matrix() const
{ return sys.data.get<0>(index)->world; }

const mat4& Transform::local_matrix() const
{ return sys.data.get<0>(index)->local; }


vec3 Transform::to_world(vec3 point) const
{
	TransformSystem::transform_t *tr = sys.data.get<0>(index);
	return vec3(tr->world * vec4(point, 1.0f));
}

vec3 Transform::vec_to_world(vec3 vec) const
{
	TransformSystem::transform_t *tr = sys.data.get<0>(index);
	return to_world(vec) - tr->position;
}
