#include <vector>

#include "UI/UI.h"
#include "Graphics/GLDriver.h"
#include "Graphics/Textures/Texture.h"

struct texture_t
{
	uvec2 size;
	const char *URI;
	uint32_t gen;
};

extern std::vector<texture_t> textures;


Texture texture_dropdown(Texture selected, const char *label)
{
	bool valid = selected.is_valid();
	uint32_t id = selected.id();
	const char *name = valid ? Texture::get(id).uri() : "None";

	if (ImGui::BeginCombo(label, name))
	{
		for (uint32_t i(1); i <= Texture::count(); i++)
		{
			Texture texture = Texture::get(i);
			if (texture == Texture::none)
				continue;

			const bool is_selected = (valid && i == id);
			if (ImGui::Selectable(texture.uri(), is_selected))
				selected = texture;
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	return selected;
}

void texture_tab(Texture *texture)
{
	*texture = texture_dropdown(*texture, "Select texture");
	ImGui::Separator();

	if (!texture->is_valid())
		return;
}

