#include <UI/UI.h>
#include <Graphics/Graphics.h>

#include "Editor/Graphics/GraphicsUI.h"

namespace GraphicsSystemUI
{

static void add_component(GraphicsSystem *sys, const char *name, Entity e)
{
	if (!ImGui::BeginMenu(name))
		return;

	if (ImGui::MenuItem("Camera"))
	{}
	if (ImGui::MenuItem("Renderer"))
	{}

	ImGui::EndMenu();
}

const system_editor_t editor = []() {
	system_editor_t e{};
	e.type_name = "GraphicsSystem";

	e.add_system = NULL;
	e.add_component = decltype(e.add_component)(add_component);
	e.edit_entity = NULL;

	return e;
}();

}
