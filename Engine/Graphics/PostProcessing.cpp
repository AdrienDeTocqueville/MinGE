#include "Profiler/profiler.h"

#include "Graphics/PostProcessing.h"

#include "Render/RenderEngine.h"
#include "Render/CommandBuffer.h"
#include "Render/Shaders/Material.inl"

#include "IO/Input.h"

static Material post_process;

PostProcessingSystem::PostProcessingSystem(void *system_dependency, Texture color_texture, float exposure, vec4 ss_viewport):
	dependency(system_dependency), ss_viewport(ss_viewport),
	enable(0), texture(color_texture), exposure(exposure)
{
	cmd_buffer = RenderEngine::create_cmd_buffer();

	if (post_process == Material::none)
		post_process = Material::create(Shader::load("asset:shader/postprocessing"));
}

PostProcessingSystem::~PostProcessingSystem()
{
	RenderEngine::destroy_cmd_buffer(cmd_buffer);
}

static void update(PostProcessingSystem *self)
{
	post_process.set("hdr", self->enable);
	post_process.set("color_buffer", self->texture);
	post_process.set("exposure", self->exposure);

	auto &cmd = RenderEngine::get_cmd_buffer(self->cmd_buffer);
	ivec2 ws = Input::window_size();
	cmd.fullscreen_pass(0, ivec4(0,0,ws.x,ws.y), post_process.id());
}

// Serialization
#include "Core/Serialization.h"

using namespace nlohmann;

PostProcessingSystem::PostProcessingSystem(const SerializationContext &ctx):
	PostProcessingSystem(ctx.get_dependency(0), Texture::get(ctx["texture"]),
		ctx["exposure"], ::to_vec4(ctx["ss_viewport"]))
{ }

void PostProcessingSystem::save(SerializationContext &ctx) const
{
	json dump = json::object();
	dump["texture"] = texture.uint();
	dump["exposure"] = exposure;
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

