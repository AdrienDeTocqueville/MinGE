#pragma once

/// Transform
void Transform::set_position(vec3 pos)
{
	sys.data.get<0>()[index].position = pos;
	sys.update_matrices(index);
}

void Transform::set_rotation(vec3 rot)
{
	sys.data.get<0>()[index].rotation = quat(rot);
	sys.update_matrices(index);
}

void Transform::set_rotation(quat rot)
{
	sys.data.get<0>()[index].rotation = rot;
	sys.update_matrices(index);
}

void Transform::set_scale(vec3 scale)
{
	sys.data.get<0>()[index].scale = scale;
	sys.update_matrices(index);
}


vec3 Transform::position() const
{ return sys.data.get<0>()[index].position; }

quat Transform::rotation() const
{ return sys.data.get<0>()[index].rotation; }

vec3 Transform::scale() const
{ return sys.data.get<0>()[index].scale; }


const mat4& Transform::world_matrix()
{ return sys.data.get<0>()[index].world; }

const mat4& Transform::local_matrix()
{ return sys.data.get<0>()[index].local; }
