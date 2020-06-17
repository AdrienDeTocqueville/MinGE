#include <UI/UI.h>
#include <Graphics/Graphics.h>

#include "Editor/Editor.h"
#include "Editor/Render/RenderUI.h"
#include "Editor/Graphics/GraphicsUI.h"

namespace GraphicsSystemUI
{

static void add_component(GraphicsSystem *sys, const char *name, Entity e)
{
	bool c = sys->has_camera(e);
	bool r = sys->has_renderer(e);
	if (c && r) return;

	if (!ImGui::BeginMenu(name))
		return;

	if (!c && ImGui::MenuItem("Camera"))
		sys->add_camera(e);
	if (!r && ImGui::MenuItem("Renderer"))
		sys->add_renderer(e, Mesh::none);

	ImGui::EndMenu();
}

struct ActionData
{
	GraphicsSystem *sys;
	Entity e;
};

#define SIMPLE_PROP(label, widget, type, owner, prop)	\
Editor::field(label, widget, owner.prop(), data,	\
(void (*)(type*,type*,ActionData*))[](type *old, type *val, ActionData *d) {	\
	if (d->sys->has_##owner(d->e)) d->sys->get_##owner(d->e).set_##prop(*val); \
});

static void edit_entity(GraphicsSystem *sys, Entity e)
{
	ActionData data { sys, e };

	// Camera
	if (sys->has_camera(e) && ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Camera camera = sys->get_camera(e);
		SIMPLE_PROP("Near plane", ImGui::DragFloat, float, camera, near_plane)
		SIMPLE_PROP("Far plane", ImGui::DragFloat, float, camera, far_plane)
		SIMPLE_PROP("Field of view", ImGui::DragFloat, float, camera, fov)
	}

	// Renderer
	if (sys->has_renderer(e) && ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Renderer renderer = sys->get_renderer(e);
		SIMPLE_PROP("Mesh", mesh_dropdown, Mesh, renderer, mesh)
	}
}

const system_editor_t editor = []() {
	system_editor_t e{};
	e.type_name = "GraphicsSystem";

	e.add_system = NULL;
	e.add_component = decltype(e.add_component)(add_component);
	e.edit_entity = decltype(e.edit_entity)(edit_entity);

	return e;
}();

}
