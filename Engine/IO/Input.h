#pragma once

#include <bitset>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "Math/glm.h"

namespace sf
{ class Window; }

struct Input
{
	enum Cursor {Free, Capture};

	// Window
	static sf::Window* window() { return win; }
	static void close_window();

	static bool  window_closed() { return closed; }
	static bool  window_focused() { return has_focus; }
	static ivec2 window_size() { return dim; }
	static void  set_window_size(ivec2 _size);

	// Keyboard
	static inline bool key_down(sf::Keyboard::Key _key);
	static inline bool key_changed(sf::Keyboard::Key _key);
	static inline bool key_pressed (sf::Keyboard::Key _key);
	static inline bool key_released(sf::Keyboard::Key _key);

	// Mouse
	static inline bool button_down(sf::Mouse::Button _button);
	static inline bool button_changed(sf::Mouse::Button _button);
	static inline bool button_pressed(sf::Mouse::Button _button);
	static inline bool button_released(sf::Mouse::Button _button);

	static int  wheel_scroll() { return mouse_wheel_delta; }
	static ivec2 mouse_delta() { return mouse_pos_delta; }
	static vec2 mouse_delta_ss() { return vec2((float)mouse_pos_delta.x / dim.x, -(float)mouse_pos_delta.y / dim.y); }


	static ivec2 mouse_position(bool openGLSpace = true) { return ivec2(mouse_pos.x, openGLSpace ? dim.y - mouse_pos.y : mouse_pos.y); }
	static vec2 mouse_position_ss();
	static void set_mouse_position(ivec2 _pos, bool openGLSpace = true);

	static Cursor cursor_mode() { return mode; }
	static void   set_cursor_mode(Cursor _mode, bool _visible = true);

private:
	/// Methods (private)
	static void init(sf::Window* _window);
	static void poll_events();

	/// Attributes (private)
	static sf::Window* win;

	static ivec2 dim, center;

	static ivec2 prev_mouse_pos, mouse_pos;
	static ivec2 mouse_pos_delta;
	static int mouse_wheel_delta;

	static Cursor mode;

	static bool has_focus;
	static bool closed;

	static int mouse_index, keyboard_index;
	static bool mouse_cleared, keyboardCleared;

	static std::bitset<sf::Mouse::ButtonCount> mouse_state[2];
	static std::bitset<sf::Keyboard::KeyCount> keyboard_state[2];

	friend struct Engine;
};

// Keyboard
bool Input::key_down(sf::Keyboard::Key _key)
{
	return keyboard_state[keyboard_index][_key];
}

bool Input::key_changed(sf::Keyboard::Key _key)
{
	return (keyboard_state[0][_key] != keyboard_state[1][_key]);
}

bool Input::key_pressed(sf::Keyboard::Key _key)
{
	return key_changed(_key) & key_down(_key);
}

bool Input::key_released(sf::Keyboard::Key _key)
{
	return key_changed(_key) & !key_down(_key);
}



// Mouse
bool Input::button_down(sf::Mouse::Button _button)
{
	return mouse_state[mouse_index][_button];
}

bool Input::button_changed(sf::Mouse::Button _button)
{
	return (mouse_state[0][_button] != mouse_state[1][_button]);
}

bool Input::button_pressed(sf::Mouse::Button _button)
{
	return button_changed(_button) & button_down(_button);
}

bool Input::button_released(sf::Mouse::Button _button)
{
	return button_changed(_button) & !button_down(_button);
}
