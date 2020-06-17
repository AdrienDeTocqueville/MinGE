#include <UI/UI.h>
#include <UI/imgui/imgui_internal.h>
#include <Memory/Memory.h>
#include <IO/Input.h>

#include "Editor.h"

static uint8_t *history = (uint8_t*)mem::alloc_page(mem::page_size);
static uint32_t h_start = 0, h_end = 0, h_head = h_start;
static ImGuiID active_item = 0;
static const int reset_bit = 1 << 30;
static const int next_reset_bit = 1 << 31;

void Editor::clear_history()
{
	h_start = h_head = h_end = 0;
}

void Editor::frame()
{
	if (ImGui::GetActiveID() == 0) active_item = 0;

	// Handle shortcuts
	if (ImGui::GetIO().WantTextInput)
		return;
	if (Input::key_down(Key::LeftControl))
	{
		if (Input::key_pressed(Key::Z) && can_undo()) undo();
		else if (Input::key_pressed(Key::Y) && can_redo()) redo();
	}
	Editor::handle_shortcuts();
}

template<typename W>
static bool is_widget(void (*w)(), W widget)
{
	return w == (void(*)())widget;
}
template<typename W, typename... Ws>
static bool is_widget(void (*w)(), W widget, Ws... widgets)
{
	return is_widget(w, widget) || is_widget(w, widgets...);
}

static uint32_t read_hist(uint32_t offset, void *x, uint32_t size)
{
	memcpy(x, history + offset, size);
	return offset + size;
}

static uint32_t write_hist(uint32_t offset, const void *x, uint32_t size)
{
	memcpy(history + offset, x, size);
	return offset + size;
}

static void advance_start()
{
	uint32_t temp1 = h_start, temp2;
	temp1 = read_hist(temp1, &temp2, sizeof(temp2));
	temp1 += temp2 * 2;
	temp1 = read_hist(temp1, &temp2, sizeof(temp2));
	temp1 += temp2;
	temp1 += sizeof(void(*)());
	h_start = read_hist(temp1, &temp2, sizeof(temp2));
	if (temp2 & reset_bit) h_start = 0;
}

static void write_hist(const char *label,
		uint32_t val_size, const void *old, const void *val,
		uint32_t data_size, const void *data,
		Editor::Action action)
{
	ImGuiID item = ImGui::GetCurrentWindowRead()->GetID(label);
	const size_t total_size = sizeof(val_size) + val_size + val_size + sizeof(data_size) + data_size + sizeof(action) + sizeof(uint32_t);

	uint32_t start;
	if (item == active_item)
	{
		// Amend last action with new value
		start = h_head - total_size + sizeof(val_size) + val_size;
		write_hist(start, val, val_size);
		action(history + start - val_size,
			history + start,
			history + start + val_size + sizeof(data_size));
		return;
	}

	start = h_head;
	active_item = item;

	if (h_head + total_size > mem::page_size)
	{
		history[h_head - sizeof(uint32_t)] |= next_reset_bit;
		start |= reset_bit;
		h_head = 0;
		if (h_head <= h_start)
			advance_start();
	}
	else if (h_head != h_start)
	{
		history[h_head - sizeof(uint32_t)] &= ~next_reset_bit;
		if (h_head < h_start && h_head + total_size >= h_start)
			advance_start();
	}

	h_head = write_hist(h_head, &val_size, sizeof(val_size));
	h_head = write_hist(h_head, old, val_size);
	h_head = write_hist(h_head, val, val_size);
	h_head = write_hist(h_head, &data_size, sizeof(data_size));
	h_head = write_hist(h_head, data, data_size);
	h_head = write_hist(h_head, &action, sizeof(action));
	h_head = write_hist(h_head, &start, sizeof(start));

	h_end = h_head;

	action(history + start + sizeof(val_size),
		history + start + sizeof(val_size) + val_size,
		history + start + sizeof(val_size) + val_size*2 + sizeof(data_size));
}

void Editor::field(const char *label, void (*widget)(),
		const void *val, size_t val_size,
		const void *data, size_t data_size,
		Editor::Action action)
{
	// duplicate original value for undo
	void *buffer = alloca(val_size);
	memcpy(buffer, val, val_size);

	bool changed = false;
	if (is_widget(widget, ImGui::DragFloat, ImGui::DragFloat2, ImGui::DragFloat3, ImGui::DragFloat4))
	{
		auto w = (bool(*)(const char*,void*,float,float,float,const char*,float))widget;
		changed = w(label, buffer, 0.1f, 0.0f, 0.0f, "%.3f", 1.0f);
	}
	else
	{
		auto w = (bool(*)(const char*,void*))widget;
		changed = w(label, buffer);
	}

	if (changed)
		write_hist(label, val_size, val, buffer, data_size, data, action);

}

bool Editor::can_undo() { return active_item == 0 && h_head != h_start; }
bool Editor::can_redo() { return active_item == 0 && h_head != h_end; }

void Editor::undo()
{
	uint32_t start, previous;
	read_hist(h_head - sizeof(uint32_t), &previous, sizeof(previous));

	start = ((previous & reset_bit) != 0) ? 0 : previous & ~next_reset_bit;

	uint32_t val_size, data_size;
	Editor::Action action;

	start = read_hist(start, &val_size, sizeof(val_size));
	void *old  = history + start; start += val_size;
	void *val  = history + start; start += val_size;
	start = read_hist(start, &data_size, sizeof(data_size));
	void *data = history + start; start += data_size;
	start = read_hist(start, &action, sizeof(action));

	action(val, old, data);

	h_head = previous & ~reset_bit;
}

void Editor::redo()
{
	uint32_t start = h_head, previous;
	if (h_head != h_start)
	{
		read_hist(h_head - sizeof(uint32_t), &previous, sizeof(previous));
		if (previous & next_reset_bit) start = 0;
	}

	uint32_t val_size, data_size;
	Editor::Action action;

	start = read_hist(start, &val_size, sizeof(val_size));
	void *old  = history + start; start += val_size;
	void *val  = history + start; start += val_size;
	start = read_hist(start, &data_size, sizeof(data_size));
	void *data = history + start; start += data_size;
	start = read_hist(start, &action, sizeof(action));

	action(old, val, data);

	h_head = start + sizeof(uint32_t);
}
