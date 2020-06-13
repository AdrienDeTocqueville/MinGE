#include <UI/UI.h>

#include "Editor/Core/CoreUI.h"

Entity entity_dropdown(Entity selected, const char *label)
{
	static const char *unnamed = "Unnamed";
	bool valid = selected.is_valid();
	uint32_t id = valid ? selected.id() : 0;
	const char *name = valid ? selected.name() ? selected.name() : unnamed : "None";

	if (ImGui::BeginCombo(label, name))
	{
		for (uint32_t i(1); i <= Entity::entities.size; i++)
		{
			auto ent = Entity::get(i);
			if (ent == Entity::none)
				continue;

			ImGui::PushID(i);
			if (ImGui::Selectable(ent.name() ? ent.name() : unnamed, i == id))
				selected = Entity::get(i);
			ImGui::PopID();
			if (i == id)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	return selected;
}
