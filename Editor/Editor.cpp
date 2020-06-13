#include <UI/UI.h>
#include <IO/Input.h>
#include <Core/Scene.h>
#include <Core/Engine.h>
#include <Transform/Transform.h>
#include <Graphics/Graphics.h>

#include "Editor.h"
#include "Core/CoreUI.h"
#include "Render/RenderUI.h"
#include "Utility/stb_sprintf.h"

#include "Transform/TransformUI.h"
#include "Graphics/GraphicsUI.h"

struct asset_tab_t
{
	const char *name;
	void (*callback)(void*);
	union {
		UID32 uid32;
		UID64 uid64;
	} selected;
};
static std::vector<asset_tab_t> asset_tabs;

static Scene scene;
static std::vector<system_editor_t> system_editors;

namespace ImGui {
ImVec2 CalcItemSize(ImVec2, float, float);
ImVec2 GetButtonSize(const char *label)
{
    const ImGuiStyle &style = ImGui::GetStyle();
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    return ImGui::CalcItemSize(ImVec2(0, 0), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
}
bool ButtonCentered(const char *label)
{
	SetCursorPosX((GetWindowSize().x - GetButtonSize(label).x) * 0.5);
	return Button(label);
}
}

const system_editor_t *get_editor(const char *type_name)
{
	for (int i = 0; i < system_editors.size(); i++)
	{
		if (0 == strcmp(system_editors[i].type_name, type_name))
			return system_editors.data() + i;
	}
	return NULL;
}

void menubar()
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Save", "Ctrl+S"))
		{
			// TODO
		}
		if (ImGui::MenuItem("Open Scene", "Ctrl+O"))
		{
			// TODO
		}

		if (ImGui::MenuItem("Exit"))
			Input::close_window();
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Undo", "CTRL+Z", false, Editor::can_undo())) Editor::undo();
		if (ImGui::MenuItem("Redo", "CTRL+Y", false, Editor::can_redo())) Editor::redo();

		ImGui::Separator();
		if (ImGui::MenuItem("Cut", "CTRL+X")) {}
		if (ImGui::MenuItem("Copy", "CTRL+C")) {}
		if (ImGui::MenuItem("Paste", "CTRL+V")) {}
		ImGui::EndMenu();
	}
}

void assets_win(uint32_t id)
{
	if (!ImGui::Begin("assets", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
		return ImGui::End();
	/*
	if (ImGui::BeginCombo("", NULL, ImGuiComboFlags_NoPreview))
	{
		for (int i = 0; i < tabs.size(); i++)
		{
			if (!tabs[i].opened && ImGui::Selectable(tabs[i].name, false))
				tabs[i].opened = true;
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	*/

	ImGui::BeginTabBar("tabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoTooltip);
	for (int i = 0; i < asset_tabs.size(); i++)
	{
		if (ImGui::BeginTabItem(asset_tabs[i].name, NULL, ImGuiTabItemFlags_None))
		{
			asset_tabs[i].callback(&asset_tabs[i].selected);
			ImGui::EndTabItem();
		}
	}
	ImGui::EndTabBar();
	ImGui::End();
}

void systems_win(uint32_t id)
{
	if (!ImGui::Begin("systems", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
		return ImGui::End();

	ImGui::BeginTabBar("tabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoTooltip);

	if (ImGui::BeginTabItem("Systems", NULL, ImGuiTabItemFlags_None))
	{
		for (int i = 0; i < scene.get_system_count(); i++)
		{
			auto &s = scene.get_systems()[i];
			auto *t = Engine::get_system_type(s.instance);
			if (ImGui::CollapsingHeader(s.name, ImGuiTreeNodeFlags_None))
			{
				// edit_system callback ?
				ImGui::Text("Hello");
			}
		}

		ImGui::Spacing();
		if (ImGui::ButtonCentered("Add System"))
			ImGui::OpenPopup("add_system_popup");
		if (ImGui::BeginPopup("add_system_popup"))
		{
			for (int i = 0; i < system_editors.size(); i++)
			{
				auto *e = &system_editors[i];
				if (ImGui::MenuItem(e->type_name, NULL, false, e->add_system != NULL))
				{
					// TODO: find a way to create a system with dependencies
					void *new_sys;
					if (e->add_system(&new_sys))
					{}
				}
			}
			ImGui::EndPopup();
		}

		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
	ImGui::End();
}

void entity_tab(Entity *entity)
{
	*entity = entity_dropdown(*entity, "Select entity");

	if (!entity->is_valid())
		return;

	for (int i = 0; i < scene.get_system_count(); i++)
	{
		auto &s = scene.get_systems()[i];
		auto *t = Engine::get_system_type(s.instance);
		auto *e = get_editor(t->name);
		if (e->edit_entity) e->edit_entity(s.instance, *entity);
	}

	ImGui::Spacing();
	if (ImGui::ButtonCentered("Add Component"))
		ImGui::OpenPopup("add_comp_popup");
	if (ImGui::BeginPopup("add_comp_popup"))
	{
		for (int i = 0; i < scene.get_system_count(); i++)
		{
			auto &s = scene.get_systems()[i];
			auto *t = Engine::get_system_type(s.instance);
			auto *e = get_editor(t->name);
			if (e->add_component) e->add_component(s.instance, s.name, *entity);
		}
		ImGui::EndPopup();
	}
}

void Editor::init()
{
	UI::set_menubar(menubar);
	UI::create_window(assets_win, 0);
	UI::create_window(systems_win, 0);

	asset_tabs.push_back((asset_tab_t){"Entity", (void(*)(void*))entity_tab, Entity::none});
	asset_tabs.push_back((asset_tab_t){"Material", (void(*)(void*))material_tab, Material::none});
	asset_tabs.push_back((asset_tab_t){"Texture", (void(*)(void*))texture_tab});
	asset_tabs.back().selected.uid64 = Texture::none;
}

void Editor::register_system_editor(const struct system_editor_t &editor)
{
	system_editors.push_back(editor);
}

void Editor::open_scene(const char *URI)
{
	Engine::clear();
	system_editors.clear();

	// Register builtin systems
	Engine::register_system_type(TransformSystem::type);
	Engine::register_system_type(GraphicsSystem::type);

	register_system_editor(TransformSystemUI::editor);
	register_system_editor(GraphicsSystemUI::editor);

	scene.load(URI);
}
