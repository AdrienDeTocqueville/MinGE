#include <UI/UI.h>

#include "Editor/Core/CoreUI.h"

bool entity_dropdown(const char *label, Entity *selected)
{
	static const char *unnamed = "Unnamed";
	bool changed = false;
	bool valid = selected->is_valid();
	uint32_t id = selected->id();
	const char *name = valid ? selected->name() ? selected->name() : unnamed : "None";

	if (ImGui::BeginCombo(label, name))
	{
		if (ImGui::Selectable("None", !valid) && valid)
		{ *selected = Entity::none; changed = true; }
		if (!valid) ImGui::SetItemDefaultFocus();

		for (uint32_t i(1); i <= Entity::entities.size; i++)
		{
			auto ent = Entity::get(i);
			if (ent == Entity::none)
				continue;

			ImGui::PushID(i);
			const bool is_selected = (valid && i == id);
			if (ImGui::Selectable(ent.name() ? ent.name() : unnamed, is_selected) && !is_selected)
			{ *selected = ent; changed = true; }
			if (is_selected)
				ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	return changed;
}
