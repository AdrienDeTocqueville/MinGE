#include "ECS/Transform.h"

void Transform::set_position(vec3 pos)
{
	system->data[index].position = pos;
	system->update_matrices(index);
}

void Transform::set_rotation(vec3 rot)
{
	system->data[index].rotation = quat(rot);
	system->update_matrices(index);
}

void Transform::set_rotation(quat rot)
{
	system->data[index].rotation = rot;
	system->update_matrices(index);
}

void Transform::set_scale(vec3 scale)
{
	system->data[index].scale = scale;
	system->update_matrices(index);
}


vec3 Transform::position()
{ return system->data[index].position; }

quat Transform::rotation()
{ return system->data[index].rotation; }

vec3 Transform::scale()
{ return system->data[index].scale; }


const mat4& Transform::world_matrix()
{ return system->data[index].world; }

const mat4& Transform::local_matrix()
{ return system->data[index].local; }
