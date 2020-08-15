#pragma once

struct GraphicsSystem;

#define SIMPLE_PROP(type, prop) \
inline type prop(); \
inline void set_##prop(type val);

#define ARRAY_PROP(type, prop) \
inline uint32_t prop##_count(); \
inline type prop(uint32_t i); \
inline void set_##prop(uint32_t i, type val);

struct Camera: UID32
{
	SIMPLE_PROP(float, near_plane)
	SIMPLE_PROP(float, far_plane)
	SIMPLE_PROP(float, fov)

	SIMPLE_PROP(Texture, depth_texture)
	SIMPLE_PROP(Texture, color_texture)

	SIMPLE_PROP(const Frustum&, frustum)

private:
	Camera(uint32_t id, uint32_t gen, GraphicsSystem &system):
		UID32(id, gen), sys(system) {}
	GraphicsSystem &sys;
	friend GraphicsSystem;
};

struct Renderer: UID32
{
	SIMPLE_PROP(Mesh, mesh)
	ARRAY_PROP(Material, material)

private:
	Renderer(uint32_t id, uint32_t gen, GraphicsSystem &system):
		UID32(id, gen), sys(system) {}
	GraphicsSystem &sys;
	friend GraphicsSystem;
};

struct Light: UID32
{
	SIMPLE_PROP(vec3, color)
	SIMPLE_PROP(float, radius)
	SIMPLE_PROP(float, intensity)

private:
	Light(uint32_t id, uint32_t gen, GraphicsSystem &system):
		UID32(id, gen), sys(system) {}
	GraphicsSystem &sys;
	friend GraphicsSystem;
};

#undef ARRAY_PROP
#undef SIMPLE_PROP
