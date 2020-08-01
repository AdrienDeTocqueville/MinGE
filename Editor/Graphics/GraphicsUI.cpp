#include <UI/UI.h>
#include <Render/Debug.h>

#include "Editor/Editor.h"
#include "Editor/Render/RenderUI.h"
#include "Editor/Graphics/GraphicsUI.h"


#include <Graphics/Graphics.h>

namespace GraphicsSystemUI
{

static void add_component(GraphicsSystem *sys, const char *name, Entity e)
{
	bool c = sys->has_camera(e);
	bool r = sys->has_renderer(e);
	bool l = sys->has_light(e);
	if (c && r && l) return;

	if (!ImGui::BeginMenu(name))
		return;

	if (!c && ImGui::MenuItem("Camera"))
		sys->add_camera(e);
	if (!r && ImGui::MenuItem("Renderer"))
		sys->add_renderer(e, Mesh::none);
	if (!l && ImGui::MenuItem("Light"))
		sys->add_point_light(e);

	ImGui::EndMenu();
}

struct ActionData
{
	GraphicsSystem *sys;
	Entity e;
};

#undef SIMPLE_PROP
#define SIMPLE_PROP(label, widget, type, owner, prop) do { \
void (*func)(type*,type*,ActionData*) = [](type *old, type *val, ActionData *d) { \
	if (d->sys->has_##owner(d->e)) d->sys->get_##owner(d->e).set_##prop(*val); \
}; \
Editor::field(label, widget, owner.prop(), data, func); \
} while (0)

static void edit_entity(GraphicsSystem *sys, Entity e)
{
	ActionData data { sys, e };

	// Camera
	if (sys->has_camera(e) && ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Camera camera = sys->get_camera(e);
		SIMPLE_PROP("Near plane", ImGui::DragFloat, float, camera, near_plane);
		SIMPLE_PROP("Far plane", ImGui::DragFloat, float, camera, far_plane);
		SIMPLE_PROP("Field of view", ImGui::DragFloat, float, camera, fov);

		Debug::frustum(camera.frustum());
	}

	// Renderer
	if (sys->has_renderer(e) && ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Renderer renderer = sys->get_renderer(e);
		SIMPLE_PROP("Mesh", mesh_dropdown, Mesh, renderer, mesh);
	}

	// Light
	if (sys->has_light(e) && ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Light light= sys->get_light(e);
		SIMPLE_PROP("Color", ImGui::ColorEdit3, vec3, light, color);
	}
}

const system_editor_t editor = []() {
	system_editor_t e = INIT_SYSTEM_EDITOR(GraphicsSystem);

	e.add_system = NULL;
	e.add_component = decltype(e.add_component)(add_component);
	e.edit_entity = decltype(e.edit_entity)(edit_entity);

	return e;
}();

}


#include <Graphics/PostProcessing.h>

namespace PostProcessingSystemUI
{

#undef SIMPLE_PROP
#define SIMPLE_PROP(label, widget, type, prop) do { \
void (*func)(type*,type*,PostProcessingSystem**) = [](type *old, type *val, PostProcessingSystem **sys) { \
	(*sys)->prop = *val; \
}; \
Editor::field(label, widget, sys->prop, sys, func); \
} while (0)

static void edit_system(PostProcessingSystem *sys)
{
	SIMPLE_PROP("Enable HDR", ImGui::Checkbox, int, enable);
	SIMPLE_PROP("Depth buffer", texture_dropdown, Texture, depth);
	SIMPLE_PROP("Color buffer", texture_dropdown, Texture, color);
	SIMPLE_PROP("Exposure", ImGui::DragFloat, float, exposure);
}

const system_editor_t editor = []() {
	system_editor_t e = INIT_SYSTEM_EDITOR(PostProcessingSystem);

	e.edit_system = decltype(e.edit_system)(edit_system);

	return e;
}();

}
