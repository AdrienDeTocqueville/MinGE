#include "Editor.h"
#include "Core/Entity.h"

#include "Graphics/GraphicsUI.h"
#include "Utility/stb_sprintf.h"

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

void entity_tab(Entity *entity)
{
	*entity = entity_dropdown(*entity, "Select entity");

	if (!entity->is_valid())
		return;
}

void Editor::init()
{
	//register_system(Graphics

	UI::create_tab("Material", material_tab, Material::none);
	UI::create_tab("Texture", texture_tab, Texture::none);

	UI::create_tab("Entity", entity_tab, Entity::none);
}
