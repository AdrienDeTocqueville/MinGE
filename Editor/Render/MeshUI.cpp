#include <UI/UI.h>

#include <Render/GLDriver.h>
#include <Render/Shaders/Shader.h>
#include <Render/Shaders/Program.h>
#include <Render/Shaders/Material.inl>

#include "Editor/Render/RenderUI.h"

bool mesh_dropdown(const char *label, Mesh *selected)
{
	bool changed = false;
	bool valid = selected->is_valid();
	uint32_t id = selected->id();
	const char *name = valid ? Mesh::get(id).uri() : "None";

	if (ImGui::BeginCombo(label, name))
	{
		if (ImGui::Selectable("None", !valid) && valid)
		{ *selected = Mesh::none; changed = true; }
		if (!valid) ImGui::SetItemDefaultFocus();

		for (uint32_t i(1); i <= Mesh::meshes.size; i++)
		{
			Mesh mesh = Mesh::get(i);
			if (mesh == Mesh::none)
				continue;

			const bool is_selected = (valid && i == id);
			if (ImGui::Selectable(mesh.uri(), is_selected) && !is_selected)
			{ *selected = mesh; changed = true; }
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	return changed;
}

void mesh_tab(Mesh *mesh)
{
	mesh_dropdown("Select mesh", mesh);

	if (!mesh->is_valid())
		return;
}