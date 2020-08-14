#include <UI/UI.h>

#include <Render/GLDriver.h>
#include <Render/Shader/Shader.h>
#include <Render/Shader/Program.h>
#include <Render/Shader/Material.inl>

#include "Editor/Render/RenderUI.h"

bool material_dropdown(const char *label, Material *selected)
{
	bool changed = false;
	bool valid = selected->is_valid();
	uint32_t id = selected->id();
	const char *name = valid ? Material::get(id).uri() : "None";

	if (ImGui::BeginCombo(label, name))
	{
		if (ImGui::Selectable("None", !valid) && valid)
		{ *selected = Material::none; changed = true; }
		if (!valid) ImGui::SetItemDefaultFocus();

		for (uint32_t i(1); i <= Material::materials.size; i++)
		{
			Material mat = Material::get(i);
			if (mat == Material::none)
				continue;

			const bool is_selected = (valid && i == id);
			if (ImGui::Selectable(mat.uri(), is_selected) && !is_selected)
			{ *selected = mat; changed = true; }
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	return changed;
}

void list_macros(material_t *material, Shader *shader)
{
	ImGui::Separator();
	ImGui::Text("Macros");

	ImGui::InputInt("Variant hash", (int*)&material->variant_hash, 1, 2, ImGuiInputTextFlags_ReadOnly);
	ImGui::InputInt("Variant index", (int*)&material->variant_idx, 1, 2, ImGuiInputTextFlags_ReadOnly);

	uint32_t hash_done = 0;
	for (const auto &it : shader->macros)
	{
		uint32_t mask = it.second.mask;
		bool checked = (material->variant_hash & mask) == it.second.id;
		if ((mask & (mask - 1)) == 0)
		{
			if (ImGui::Checkbox(it.first.c_str(), &checked))
			{
				uint32_t hash = (material->variant_hash & ~mask);
				if (checked) hash |= it.second.id;
				material->update_variant(hash);
			}
		}
		else if ((hash_done & mask) == 0)
		{
			hash_done |= mask;

			const char *name = NULL;
			for (const auto &it2 : shader->macros)
			{
				if ((material->variant_hash & mask) == it2.second.id)
				{ name = it2.first.c_str(); break; }
			}

			if (ImGui::BeginCombo(name, NULL, ImGuiComboFlags_NoPreview))
			{
				for (const auto &it2 : shader->macros)
				{
					if (it2.second.mask != mask) continue;
					checked = (it2.first.c_str() == name);
					if (ImGui::Selectable(it2.first.c_str(), checked))
					{
						uint32_t hash = (material->variant_hash & ~mask) | it2.second.id;
						material->update_variant(hash);
					}
					if (checked)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
	}
}

void list_uniforms(material_t *material, Shader *shader)
{
	ImGui::Separator();
	ImGui::Text("Uniforms");

	auto *passes = shader->variants[material->variant_idx].passes;
	for (const auto &it : shader->uniforms_names)
	{
		Program::Uniform *uniform = NULL;
		for (int p = 0; p < RenderPass::Count; p++)
		{
			if (passes[p] != NULL)
			for (auto &uni : passes[p]->uniforms)
			{
				if (uni.offset == it.second)
				{
					uniform = &uni;
					goto display_uniform;
				}
			}
		}
		continue;

display_uniform:
		const void *data = material->uniforms.data() + uniform->offset;
		Texture *t = (Texture*)data;

		switch (uniform->type)
		{
		case GL_FLOAT:
			ImGui::DragFloat(it.first.c_str(), (float*)data, 0.1f);
			break;
		case GL_FLOAT_VEC2:
			ImGui::DragFloat2(it.first.c_str(), (float*)data, 0.1f);
			break;
		case GL_FLOAT_VEC3:
			ImGui::DragFloat3(it.first.c_str(), (float*)data, 0.1f);
			break;
		case GL_FLOAT_VEC4:
			ImGui::DragFloat4(it.first.c_str(), (float*)data, 0.1f);
			break;
		case GL_FLOAT_MAT3:
		case GL_FLOAT_MAT4:
			break;
		case GL_SAMPLER_2D:
			texture_dropdown(it.first.c_str(), t);
			break;
		}
	}
}

void material_tab(Material *material)
{
	material_dropdown("Select material", material);

	if (!material->is_valid())
		return;

	material_t *selected = Material::materials.get<0>(material->id());
	Shader *shader = selected->shader;

	if (ImGui::Button("Reload"))
	{
		Material::reload(shader);
		shader = selected->shader;
	}

	if (shader->macros.size())
		list_macros(selected, shader);

	if (shader->uniforms_names.size())
		list_uniforms(selected, shader);
}
