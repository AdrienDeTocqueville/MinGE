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

	sys.update_submeshes(x, Material::none, true);
}

uint32_t Renderer::material_count()
{
	auto x = sys.indices.get<1>(id());
	auto *r = sys.renderers.get<0>() + x;
	return r->last_submesh - r->first_submesh;
}
Material Renderer::material(uint32_t i)
{
	auto x = sys.indices.get<1>(id());
	auto *r = sys.renderers.get<0>() + x;
	return Material::get(sys.submeshes[r->first_submesh + i].material);
}
void Renderer::set_material(uint32_t i, Material m)
{
	auto x = sys.indices.get<1>(id());
	auto *r = sys.renderers.get<0>() + x;
	sys.submeshes[r->first_submesh + i].material = m.id();
}


// Light

#define SIMPLE_LIGHT_PROP(type, prop) \
type Light::prop() {				\
	auto x = sys.indices.get<2>(id());	\
	return sys.point_lights.get<0>()[x].prop;	\
} \
void Light::set_##prop(type val) {		\
	auto x = sys.indices.get<2>(id());	\
	sys.point_lights.get<0>()[x].prop = val;	\
}

SIMPLE_LIGHT_PROP(vec3, color)
SIMPLE_LIGHT_PROP(float, radius)
SIMPLE_LIGHT_PROP(float, intensity)

#undef SIMPLE_LIGHT_PROP
