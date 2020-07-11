#pragma once

struct GraphicsSystem;

#define SIMPLE_PROP(type, prop) \
inline type prop(); \
inline void set_##prop(type val);

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

private:
	Renderer(uint32_t id, uint32_t gen, GraphicsSystem &system):
		UID32(id, gen), sys(system) {}
	GraphicsSystem &sys;
	friend GraphicsSystem;
};

struct Light: UID32
{
private:
	Light(uint32_t id, uint32_t gen, GraphicsSystem &system):
		UID32(id, gen), sys(system) {}
	GraphicsSystem &sys;
	friend GraphicsSystem;
};

#undef SIMPLE_PROP
