#pragma once

#include <Core/UID.h>

struct Editor
{
	typedef void (*Action)(void*, void*, void*);
	typedef void (*AssetEditor)(UID32*);

	static void load();
	static void clear();
	static void frame();
	static void register_system_editor(const struct system_editor_t &editor);
	static void register_asset_editor(const char *name, AssetEditor editor);

	static void open_scene(const char *path);
	static void save_scene();
	static void save_scene_as(const char *path);

	static void field(const char *label, void (*widget)(),
		const void *val, size_t val_size,
		const void *data, size_t data_size,
		Action action);

	template<typename Widget, typename V, typename D>
	static void field(const char *label, Widget widget,
		const V &val, const D &data,
		void (*action)(V*, V*, D*))
	{
		field(label, (void(*)())widget,
			&val, sizeof(V),
			&data, sizeof(D),
			(Action)action);
	}

	static bool can_undo();
	static bool can_redo();
	static void undo();
	static void redo();

private:
	static void clear_history();
	static void handle_shortcuts();
};
