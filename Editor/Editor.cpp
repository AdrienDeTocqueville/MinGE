#include <UI/UI.h>
#include <IO/Input.h>
#include <Core/Scene.h>
#include <Core/Engine.h>
#include <Transform/Transform.h>
#include <Graphics/Graphics.h>
#include <Graphics/PostProcessing.h>

#include "Editor.h"
#include "Core/CoreUI.h"
#include "Render/RenderUI.h"
#include "Utility/stb_sprintf.h"

#include "Transform/TransformUI.h"
#include "Graphics/GraphicsUI.h"

static char scene_path[255] = "";

struct asset_tab_t
{
	const char *name;
	void (*callback)(void*);
	UID32 selected;
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
	SetCursorPosX((GetWindowSize().x - GetButtonSize(label).x) * 0.5f);
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
		if (ImGui::MenuItem("Save", "Ctrl+S")) Editor::save_scene();
		if (ImGui::MenuItem("Save As", "Ctrl+Shit+S"))
		{ }
		if (ImGui::MenuItem("Open Scene", "Ctrl+O"))
		{ Editor::open_scene(/*placeholder*/scene_path); }

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
			auto *e = get_editor(t->name);
			if (e->edit_system && ImGui::CollapsingHeader(s.name, ImGuiTreeNodeFlags_None))
			{
				e->edit_system(s.instance);
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

bool add_entity(char *buf, size_t size)
{
	ImGui::PushID("new_entity_name");
	bool ret = ImGui::InputText("", buf, size, ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::PopID();
	ImGui::SameLine();
	return ImGui::Button("+") || ret;
}

void entity_tab(Entity *entity)
{
	entity_dropdown("Select entity", entity);

	static char new_name[64];
	if (add_entity(new_name, sizeof(new_name)))
	{
		*entity = Entity::create(new_name);
		*new_name = 0;
	}

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
}

void Editor::register_system_editor(const struct system_editor_t &editor)
{
	system_editors.push_back(editor);
}

void Editor::open_scene(const char *path)
{
	clear_history();
	Engine::clear();
	system_editors.clear();
	asset_tabs.clear();

	// Register builtin system types
	Engine::register_system_type(TransformSystem::type);
	Engine::register_system_type(GraphicsSystem::type);
	Engine::register_system_type(PostProcessingSystem::type);

	register_system_editor(TransformSystemUI::editor);
	register_system_editor(GraphicsSystemUI::editor);
	register_system_editor(PostProcessingSystemUI::editor);

	// Register builtin asset types
	Engine::register_asset_type(Mesh::type);
	Engine::register_asset_type(Texture::type);
	Engine::register_asset_type(Material::type);

	asset_tabs.push_back(asset_tab_t {"Entity", (void(*)(void*))entity_tab, Entity::none});
	asset_tabs.push_back(asset_tab_t {"Mesh", (void(*)(void*))mesh_tab, Mesh::none});
	asset_tabs.push_back(asset_tab_t {"Texture", (void(*)(void*))texture_tab, Texture::none});
	asset_tabs.push_back(asset_tab_t {"Material", (void(*)(void*))material_tab, Material::none});

	if (scene.load(path))
	{
		Input::set_window_name(strrchr(path, '/') + 1);
		strncpy(scene_path, path, sizeof(scene_path));

		GraphicsSystem *g = (GraphicsSystem*)scene.get_system("graphics");
		//g->add_renderer(Entity::get("Light"), Mesh::get(1));
	}
	else
	{
		scene_path[0] = 0;
		Input::set_window_name("Unnamed Scene");
	}
}

void Editor::save_scene()
{
	printf("saving scene as %s\n", scene_path);
	scene.save(scene_path);
}

void Editor::save_scene_as(const char *path)
{
	if (scene.save(path, Scene::Overwrite::Ask))
		strncpy(scene_path, path, sizeof(scene_path));
}

void Editor::handle_shortcuts()
{
	if (Input::key_down(Key::LeftControl))
	{
		if (Input::key_pressed(Key::S)) save_scene();
		//else if (Input::key_pressed(Key::O)) open_scene(/**/);
		else if (Input::key_down(Key::LeftShift))
		{
			//if (Input::key_pressed(Key::S)) save_scene_as();
		}
	}
}
