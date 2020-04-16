#pragma once

#include <bitset>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "Math/glm.h"

namespace sf
{ class RenderWindow; }

class Input
{
friend class Engine;

public:
	enum Cursor {Free, FreeHidden, Capture};

	// Window
	static sf::RenderWindow* window() { return win; }
	static void close_window();

	static bool  window_closed() { return closed; }
	static bool  window_focused() { return has_focus; }
	static ivec2 window_size() { return dim; }
	static void  set_window_size(ivec2 _size);

	// Keyboard
	static inline bool key_down(sf::Keyboard::Key _key);
	static inline bool key_pressed (sf::Keyboard::Key _key);
	static inline bool key_released(sf::Keyboard::Key _key);

	// Mouse
	static inline bool button_down(sf::Mouse::Button _button);
	static inline bool button_pressed(sf::Mouse::Button _button);
	static inline bool button_released(sf::Mouse::Button _button);

	static int  mouse_scroll() { return mouse_wheel_delta; }
	static vec2 mouse_delta() { return mouse_pos_delta; }
	static vec2 mouse_delta_relative();

	static vec2 mouse_position(bool openGLSpace = true);
	static vec2 mouse_position_relative();
	static void set_mouse_position(vec2 _pos, bool openGLSpace = true);

	static Cursor cursor_mode() { return mode; }
	static void   set_cursor_mode(Cursor _mode);

private:
	/// Methods (private)
	static void init(sf::RenderWindow* _window);
	static void update();

	/// Attributes (private)
	static sf::RenderWindow* win;

	static ivec2 dim, center;

	static vec2 prev_mouse_pos, mouse_pos;
	static vec2 mouse_pos_delta;
	static int mouse_wheel_delta;

	static Cursor mode;

	static bool has_focus;
	static bool closed;

	static int mouse_index, keyboard_index;
	static bool mouse_cleared, keyboardCleared;

	static std::bitset<sf::Mouse::ButtonCount> mouse_state[2];
	static std::bitset<sf::Keyboard::KeyCount> keyboard_state[2];
};

// Keyboard
bool Input::key_down(sf::Keyboard::Key _key)
{
	return keyboard_state[keyboard_index][_key];
}

bool Input::key_pressed(sf::Keyboard::Key _key)
{
	return (keyboard_state[keyboard_index][_key] == keyboard_state[1-keyboard_index][_key]) &&
		keyboard_state[keyboard_index][_key];
}

bool Input::key_released(sf::Keyboard::Key _key)
{
	return (keyboard_state[keyboard_index][_key] == keyboard_state[1-keyboard_index][_key]) &&
		!keyboard_state[keyboard_index][_key];
}

// Mouse
bool Input::button_down(sf::Mouse::Button _button)
{
	return mouse_state[mouse_index][_button];
}

bool Input::button_pressed(sf::Mouse::Button _button)
{
	return (mouse_state[mouse_index][_button] == mouse_state[1-mouse_index][_button]) &&
		mouse_state[mouse_index][_button];
}

bool Input::button_released(sf::Mouse::Button _button)
{
	return (mouse_state[mouse_index][_button] == mouse_state[1-mouse_index][_button]) &&
		!mouse_state[mouse_index][_button];
}
