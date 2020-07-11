#pragma once

#include "Render/CommandBuffer.h"

// Camera

#define SIMPLE_CAM_GETTER(type, prop) \
type Camera::prop() {				\
	auto x = sys.indices.get<0>(id());	\
	return sys.cameras.get<0>()[x].prop;	\
}

#define SIMPLE_CAM_SETTER(type, prop) \
void Camera::set_##prop(type val) {		\
	auto x = sys.indices.get<0>(id());	\
	sys.cameras.get<0>()[x].prop = val;	\
	sys.update_projection(x);		\
}

#define SIMPLE_CAM_PROP(type, prop) \
	SIMPLE_CAM_GETTER(type, prop) \
	SIMPLE_CAM_SETTER(type, prop)


SIMPLE_CAM_PROP(float, near_plane)
SIMPLE_CAM_PROP(float, far_plane)
SIMPLE_CAM_PROP(float, fov)
SIMPLE_CAM_GETTER(Texture, depth_texture)
SIMPLE_CAM_GETTER(Texture, color_texture)
SIMPLE_CAM_GETTER(const Frustum&, frustum)


#undef SIMPLE_CAM_PROP
#undef SIMPLE_CAM_SETTER
#undef SIMPLE_CAM_GETTER

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
