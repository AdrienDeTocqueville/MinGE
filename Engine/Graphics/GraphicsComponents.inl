#pragma once

#include "Render/CommandBuffer.h"

// Camera

#define SIMPLE_CAM_PROP(type, prop) \
type Camera::prop() {				\
	auto x = sys.indices.get<0>(id());	\
	return sys.cameras.get<0>()[x].prop;	\
}\
void Camera::set_##prop(type val) {	\
	auto x = sys.indices.get<0>(id());	\
	sys.cameras.get<0>()[x].prop = val;	\
	sys.update_projection(x);		\
}

SIMPLE_CAM_PROP(float, near_plane)
SIMPLE_CAM_PROP(float, far_plane)
SIMPLE_CAM_PROP(float, fov)
SIMPLE_CAM_PROP(Texture, color_texture)

#undef SIMPLE_CAM_PROP

// Renderer
Mesh Renderer::mesh()
{
	auto x = sys.indices.get<1>(id());
	return sys.renderers.get<0>()[x].mesh;
}
void Renderer::set_mesh(Mesh val)
{
	auto x = sys.indices.get<1>(id());
	sys.renderers.get<0>()[x].mesh = val;

	sys.update_submeshes(x, true);
}
