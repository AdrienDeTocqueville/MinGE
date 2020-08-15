#include "Profiler/profiler.h"

#include "Graphics/PostProcessing.h"

#include "Render/RenderEngine.h"
#include "Render/CommandBuffer.h"
#include "Render/Shader/Material.inl"

#include "IO/Input.h"

PostProcessingSystem::PostProcessingSystem(void *system_dependency, Texture depth_texture, Texture color_texture, vec4 ss_viewport, Material post_processing):
	dependency(system_dependency), ss_viewport(ss_viewport),
	enable(1), depth(depth_texture), color(color_texture),
	post_process(post_processing)
{
	if (post_process == Material::none)
		post_process = Material::load("asset:material?shader=postprocessing");

	RenderEngine::add_buffer(&cmd_buffer);
}

PostProcessingSystem::~PostProcessingSystem()
{
	RenderEngine::remove_buffer(&cmd_buffer);
}

static void update(PostProcessingSystem *self)
{
	Material post_process = self->post_process;
	post_process.set("hdr", self->enable);
	post_process.set("depth_buffer", self->depth);
	post_process.set("color_buffer", self->color);

	ivec2 ws = Input::window_size();

	cmd_buffer_t &cmd = self->cmd_buffer;
	cmd.fullscreen_pass(0, ivec4(0,0,ws.x,ws.y), post_process.id());
}

// Serialization
#include "Core/Serialization.h"

using namespace nlohmann;

PostProcessingSystem::PostProcessingSystem(const SerializationContext &ctx):
	PostProcessingSystem(ctx.get_dependency(0), Texture::get(ctx["depth"]), Texture::get(ctx["color"]), ::to_vec4(ctx["ss_viewport"]), Material::get(ctx["material"]))
{ }

void PostProcessingSystem::save(SerializationContext &ctx) const
{
	json dump = json::object();
	dump["depth"] = depth.uint();
	dump["color"] = color.uint();
	dump["material"] = post_process.uint();
	dump["ss_viewport"] = ::to_json(ss_viewport);

	ctx.set_dependencies(dependency);
	ctx.swap_data(dump);
}

// Type definition
const system_type_t PostProcessingSystem::type = []() {
	system_type_t t = INIT_SYSTEM_TYPE(PostProcessingSystem);

	t.destroy = [](void *system) { ((PostProcessingSystem*)system)->~PostProcessingSystem(); };
	t.update = (decltype(t.update))update;

	t.save = [](void *system, SerializationContext &ctx) { ((PostProcessingSystem*)system)->save(ctx); };
	t.load = [](void *system, const SerializationContext &ctx) { new(system) PostProcessingSystem(ctx); };
	return t;
}();

