#include "UI/UI.h"

#include "Graphics/GLDriver.h"
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Shaders/Program.h"
#include "Graphics/Shaders/Material.inl"
#include "Graphics/Textures/Texture.h"

void material_tab()
{
	// Should not be static
	static material_t *selected = NULL;

	if (ImGui::BeginCombo("Select material", NULL, ImGuiComboFlags_NoPreview))
	{
		for (uint32_t i(1); i <= Material::materials.size; i++)
		{
			material_t *mat = Material::materials.get<0>(i);
			if (mat->shader == NULL)
				continue;
			const bool is_selected = (selected == mat);
			if (ImGui::Selectable(mat->shader->URI, is_selected))
				selected = mat;
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (selected == NULL)
		return;

	ImGui::Text(selected->shader->URI);
	ImGui::Separator();

	ImGui::InputInt("Variant hash", (int*)&selected->variant_hash, 1, 2, ImGuiInputTextFlags_ReadOnly);
	ImGui::InputInt("Variant index", (int*)&selected->variant_idx, 1, 2, ImGuiInputTextFlags_ReadOnly);

	ImGui::Text("Macros");

	uint32_t hash_done = 0;
	Shader *shader = selected->shader;
	for (const auto &it : shader->macros)
	{
		uint32_t mask = it.second.mask;
		bool checked = (selected->variant_hash & mask) == it.second.id;
		if ((mask & (mask - 1)) == 0)
		{
			if (ImGui::Checkbox(it.first.c_str(), &checked))
			{
				uint32_t hash = (selected->variant_hash & ~mask);
				if (checked) hash |= it.second.id;
				selected->update_variant(hash);
			}
		}
		else if ((hash_done & mask) == 0)
		{
			hash_done |= mask;

			const char *name = NULL;
			for (const auto &it2 : shader->macros)
			{
				if ((selected->variant_hash & mask) == it2.second.id)
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
						uint32_t hash = (selected->variant_hash & ~mask) | it2.second.id;
						selected->update_variant(hash);
					}
					if (checked)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
	}

	ImGui::Separator();
	ImGui::Text("Uniforms");

	auto *passes = shader->variants[selected->variant_idx].passes;
	for (const auto &it : shader->uniforms_names)
	{
		Program::Uniform *uniform = NULL;
		for (int p = 0; p < RenderPass::Count; p++)
		{
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
		const void *data = selected->uniforms.data() + uniform->offset;
		Texture *t = (Texture*)data;

		switch (uniform->type)
		{
		case GL_FLOAT:
			ImGui::SliderFloat(it.first.c_str(), (float*)data, 0.0f, 1.0f);
			break;
		case GL_FLOAT_VEC2:
			ImGui::SliderFloat2(it.first.c_str(), (float*)data, 0.0f, 1.0f);
			break;
		case GL_FLOAT_VEC3:
			ImGui::SliderFloat3(it.first.c_str(), (float*)data, 0.0f, 1.0f);
			break;
		case GL_FLOAT_VEC4:
			ImGui::SliderFloat4(it.first.c_str(), (float*)data, 0.0f, 1.0f);
			break;
		case GL_FLOAT_MAT3:
		case GL_FLOAT_MAT4:
			break;
		case GL_SAMPLER_2D:
			break;
		}
	}
}

