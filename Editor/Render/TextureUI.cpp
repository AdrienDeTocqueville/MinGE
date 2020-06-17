#include <vector>

#include <UI/UI.h>
#include <Render/GLDriver.h>
#include <Render/Textures/Texture.h>

struct texture_t
{
	uvec2 size;
	const char *URI;
	uint32_t gen;
};

extern std::vector<texture_t> textures;


bool texture_dropdown(const char *label, Texture *selected)
{
	bool changed = false;
	bool valid = selected->is_valid();
	uint32_t id = selected->id();
	const char *name = valid ? Texture::get(id).uri() : "None";

	if (ImGui::BeginCombo(label, name))
	{
		if (ImGui::Selectable("None", !valid) && valid)
		{ *selected = Texture::none; changed = true; }
		if (!valid) ImGui::SetItemDefaultFocus();

		for (uint32_t i(1); i <= Texture::count(); i++)
		{
			Texture texture = Texture::get(i);
			if (texture == Texture::none)
				continue;

			const bool is_selected = (valid && i == id);
			if (ImGui::Selectable(texture.uri(), is_selected) && !is_selected)
			{ *selected = texture; changed = true; }
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	return changed;
}

void texture_tab(Texture *texture)
{
	texture_dropdown("Select texture", texture);
	ImGui::Separator();

	if (!texture->is_valid())
		return;
}

