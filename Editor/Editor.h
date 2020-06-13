#pragma once

struct Editor
{
	static void init();
	static void frame();
	static void register_system_editor(const struct system_editor_t &editor);

	static void open_scene(const char *URI);

	static void field(const char *label, void (*widget)(),
			void *val, size_t val_size,
			void *data, size_t data_size,
			void (*action)(void*, void*, void*));

	template<typename Widget, typename V, typename D>
	static void field(const char *label, Widget widget, V &val, D &data,
			void (*action)(V*, V*, D*))
	{
		field(label, (void(*)())widget, &val, sizeof(V), &data, sizeof(D),
			(void(*)(void*, void*, void*))action);
	}

	static bool can_undo();
	static bool can_redo();
	static void undo();
	static void redo();

private:
	void clear_history();
};
