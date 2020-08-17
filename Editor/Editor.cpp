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
	void (*callback)(UID32*);
	UID32 selected;
};
static std::vector<asset_tab_t> asset_tabs;

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

static const system_editor_t *get_editor(const char *type_name)
{
	for (int i = 0; i < system_editors.size(); i++)
	{
		if (0 == strcmp(system_editors[i].type_name, type_name))
			return system_editors.data() + i;
	}
	return NULL;
}

static void save_as_dialog()
{
	if (choose_file("Save As", scene_path, ARRAY_LEN(scene_path), true))
		Editor::save_scene_as(scene_path);
}

static void open_scene_dialog()
{
	if (choose_file("Open Scene", scene_path, ARRAY_LEN(scene_path), false))
		Editor::open_scene(scene_path);
}

static void menubar()
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New Scene", "Ctrl+N"))	Editor::open_scene(NULL);
		if (ImGui::MenuItem("Save", "Ctrl+S"))		Editor::save_scene();
		if (ImGui::MenuItem("Save As", "Ctrl+Shit+S"))	save_as_dialog();
		if (ImGui::MenuItem("Open Scene", "Ctrl+O"))	open_scene_dialog();

		if (ImGui::MenuItem("Exit")) Input::close_window();
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

static void assets_win(uint32_t id)
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

static void systems_win(uint32_t id)
{
	if (!ImGui::Begin("systems", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
		return ImGui::End();

	ImGui::BeginTabBar("tabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoTooltip);

	if (ImGui::BeginTabItem("Systems", NULL, ImGuiTabItemFlags_None))
	{
		for (int i = 0; i < Scene::system_count(); i++)
		{
			auto &s = Scene::systems()[i];
			auto *t = Engine::system_type(s.instance);
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

static bool add_entity(char *buf, size_t size)
{
	ImGui::PushID("new_entity_name");
	bool ret = ImGui::InputText("", buf, size, ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::PopID();
	ImGui::SameLine();
	return ImGui::Button("+") || ret;
}

static void entity_tab(Entity *entity)
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

	for (int i = 0; i < Scene::system_count(); i++)
	{
		auto &s = Scene::systems()[i];
		auto *t = Engine::system_type(s.instance);
		auto *e = get_editor(t->name);
		if (e->edit_entity) e->edit_entity(s.instance, *entity);
	}

	ImGui::Spacing();
	if (ImGui::ButtonCentered("Add Component"))
		ImGui::OpenPopup("add_comp_popup");
	if (ImGui::BeginPopup("add_comp_popup"))
	{
		for (int i = 0; i < Scene::system_count(); i++)
		{
			auto &s = Scene::systems()[i];
			auto *t = Engine::system_type(s.instance);
			auto *e = get_editor(t->name);
			if (e->add_component) e->add_component(s.instance, s.name, *entity);
		}
		ImGui::EndPopup();
	}
}

void Editor::load()
{
	UI::set_menubar(menubar);
	UI::create_window(assets_win, 0);
	UI::create_window(systems_win, 0);

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

	register_asset_editor("Entity",	  (AssetEditor)entity_tab);
	register_asset_editor("Mesh",	  (AssetEditor)mesh_tab);
	register_asset_editor("Texture",  (AssetEditor)texture_tab);
	register_asset_editor("Material", (AssetEditor)material_tab);
}

void Editor::clear()
{
	clear_history();
	system_editors.clear();
	asset_tabs.clear();
}

void Editor::register_system_editor(const struct system_editor_t &editor)
{
	system_editors.push_back(editor);
}

void Editor::register_asset_editor(const char *name, AssetEditor editor)
{
	asset_tabs.push_back(asset_tab_t {name, editor, UID32()});
}

void Editor::open_scene(const char *path)
{
	Engine::clear();
	Editor::clear();

	Editor::load();

	if (path && Scene::load(path))
	{
		Input::set_window_name(strrchr(path, '/') + 1);
		if (path != scene_path)
			strncpy(scene_path, path, sizeof(scene_path));
	}
	else
	{
		scene_path[0] = 0;
		Engine::load();
		Input::set_window_name("Unnamed Scene");

		auto transforms = new(Engine::alloc_system("TransformSystem"))
			TransformSystem();
		auto graphics = new(Engine::alloc_system("GraphicsSystem"))
			GraphicsSystem(transforms);

		Entity mesh_ent = Entity::create("Cube");
		transforms->add(mesh_ent);
		graphics->add_renderer(mesh_ent, Mesh::load("asset:mesh/cube"));

		Entity light_ent = Entity::create("Light");
		transforms->add(light_ent, vec3(-0.6, -3, 2));
		graphics->add_point_light(light_ent);

		Entity camera_ent = Entity::create("MainCamera");
		transforms->add(camera_ent, vec3(-5, -3, 3), radians(vec3(0, 13, 15)));
		auto cam = graphics->add_camera(camera_ent);

		auto postproc = new(Engine::alloc_system("PostProcessingSystem"))
			PostProcessingSystem(graphics, cam.depth_texture(), cam.color_texture());

		Scene::set_systems(
			"transforms", transforms,
			"graphics", graphics,
			"post-processing", postproc
		);
	}
}

void Editor::save_scene()
{
	if (*scene_path)
		Scene::save(scene_path);
	else
		save_as_dialog();
}

void Editor::save_scene_as(const char *path)
{
	if (Scene::save(path, Scene::Overwrite::Ask))
		strncpy(scene_path, path, sizeof(scene_path));
}

void Editor::handle_shortcuts()
{
	if (Input::key_down(Key::LeftControl))
	{
		if (Input::key_down(Key::LeftShift))
		{
			if (Input::key_pressed(Key::S)) save_as_dialog();
		}
		else
		{
			if (Input::key_pressed(Key::N)) open_scene(NULL);
			else if (Input::key_pressed(Key::S)) save_scene();
			else if (Input::key_pressed(Key::O)) open_scene_dialog();
		}
	}
}
