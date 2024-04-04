#include "Profiler/profiler.h"
#include "Graphics/Graphics.h"
#include "Render/GLDriver.h"
#include "Render/CommandBuffer.h"
#include "Render/RenderEngine.h"
#include "IO/Input.h"
#include "Utility/stb_sprintf.h"
#include "Utility/Log.h"

uint32_t GraphicsSystem::init_camera(Entity entity, float fov, float near_plane, float far_plane,
	bool orthographic, vec2 size_scale, vec4 clear_color, Texture color, Texture depth)
{
	uint32_t i = cameras.add();
	indices.map<0>(entity, i);

	camera_t *cam = cameras.get<0>() + i;

	cam->near_plane	= near_plane;
	cam->far_plane	= far_plane;
	cam->clear_color= clear_color;
	cam->fbo_depth  = 0;
	cam->fbo_forward= 0;
	cam->screen_size = vec2(0);
	cam->fov	= fov;
	cam->entity	= entity;
	cam->size_scale = size_scale;
	cam->ortho	= orthographic;
	cam->color_texture = color;
	cam->depth_texture = depth;

	return i;
}

Renderer GraphicsSystem::add_renderer(Entity entity, Mesh mesh, Material material)
{
	uint32_t i = renderers.add();
	indices.map<1>(entity, i);

	renderers.get<0>()[i].entity = entity;
	renderers.get<0>()[i].mesh = mesh;

	update_submeshes(i, material, false);

	return {entity.id(), 0, *this};
}

Light GraphicsSystem::add_point_light(Entity entity, vec3 color, float intensity, float radius)
{
	uint32_t i = point_lights.add();
	indices.map<2>(entity, i);

	point_lights.get<0>()[i].color = color;
	point_lights.get<0>()[i].intensity = intensity;
	point_lights.get<0>()[i].radius = radius;
	point_lights.get<0>()[i].entity = entity;

	return { entity.id(), 0, *this};
}


GLuint validate_framebuffer(GLuint fbo)
{
	int val = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (val != GL_FRAMEBUFFER_COMPLETE)
	{
		static char err[256];
		const char *str;
		switch (val)
		{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: str = "Incomplete attachment"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: str = "Incomplete draw buffer"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: str = "Incomplete read buffer"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: str = "Missing attachment"; break;
		case GL_FRAMEBUFFER_UNSUPPORTED: str = "Framebuffer unsupported"; break;
		default: str = "Unknow error code"; break;
		}
		stbsp_snprintf(err, sizeof(err), "Failed to create framebuffer -> %s (%d).", str, val);
		Log::add(Log::Error, err);
		GL::DeleteFramebuffer(fbo);
		return 0;
	}
	return fbo;
}

GLuint create_framebuffer(render_texture_t depth_buffer, GLuint color_texture)
{
	GLuint fbo = GL::GenFramebuffer();
	GL::BindFramebuffer(fbo);

	GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
	glCheck(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0));
	glCheck(glDrawBuffers(1, draw_buffers));

	glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer.handle));

	return validate_framebuffer(fbo);
}

static inline bool is_smaller(const vec2 &a, const vec2 &b)
{
	return a.x < b.x || a.y < b.y;
}

void GraphicsSystem::resize_rt(uint32_t i)
{
	camera_t *cam = cameras.get<0>() + i;

	vec2 ws = Input::window_size();
	vec2 size_scale = cam->size_scale;
	ivec2 resolution = ivec2(size_scale.x * ws.x, size_scale.y * ws.y);

	if (resolution == cam->screen_size)
		return;

	cam->screen_size = resolution;

	cam->depth_buffer.create(resolution, render_texture_t::Format::DEPTH24_STENCIL8);

	Texture::settings_t tex_settings;
	tex_settings.wrap_mode_s = tex_settings.wrap_mode_t = GL::ClampToEdge;
	tex_settings.filter_mode = GL::Point;
	tex_settings.width = resolution.x;
	tex_settings.height = resolution.y;

	// Depth prepass
	tex_settings.format = GL_R32F;
	if (!cam->depth_texture.valid()) cam->depth_texture = Texture::create(tex_settings);
	//else if (is_smaller(cam->depth_texture.size(), resolution)) cam->depth_texture.reload(uri);

	if (cam->fbo_depth) GL::DeleteFramebuffer(cam->fbo_depth);
	cam->fbo_depth = create_framebuffer(cam->depth_buffer, cam->depth_texture.handle());

	// Forward pass
	tex_settings.format = GL_RGBA16F;
	if (!cam->color_texture.valid()) cam->color_texture = Texture::create(tex_settings);
	//else if (is_smaller(cam->color_texture.size(), resolution)) cam->color_texture.reload(uri);

	if (cam->fbo_forward) GL::DeleteFramebuffer(cam->fbo_forward);
	cam->fbo_forward = create_framebuffer(cam->depth_buffer, cam->color_texture.handle());

	// Compute projection matrix
	update_projection(i);
}

void GraphicsSystem::update_projection(uint32_t i)
{
	camera_t *cam = cameras.get<0>() + i;
	vec2 res = cam->screen_size;

	if (cam->ortho)
	{
		float half_width = cam->fov * 0.5f;
		float half_height = half_width * res.y / res.x;
		cam->projection = ortho(-half_width, half_width, -half_height, half_height, cam->near_plane, cam->far_plane);
	}
	else
	{
		float aspect_ratio = res.x / res.y;
		cam->projection = perspective(glm::radians(cam->fov), aspect_ratio,
			cam->near_plane, cam->far_plane);
	}
}

void GraphicsSystem::update_submeshes(uint32_t i, Material material, bool remove_previous)
{
	renderer_t *r = renderers.get<0>() + i;

	if (remove_previous && r->first_submesh != submeshes.invalid_id())
		submeshes.remove(r->first_submesh, r->last_submesh - r->first_submesh);

	if (r->mesh.valid())
	{
		if (!material.valid())
			material = RenderEngine::default_material;

		Mesh::submeshes_t subs = r->mesh.submesh_list();
		uint32_t first = submeshes.add(subs.count);

		for (uint32_t s = 0; s < subs.count; s++)
		{
			submeshes[first+s].submesh = Mesh::submeshes[subs.first + s];
			submeshes[first+s].material = material.id();
			submeshes[first+s].renderer = i - 1;
		}

		r->first_submesh = first;
		r->last_submesh = first + subs.count;
	}
	else
		r->first_submesh = r->last_submesh = submeshes.invalid_id();

	// Mark draw_order_indices as dirty
	objects.capacity |= -1;
}
