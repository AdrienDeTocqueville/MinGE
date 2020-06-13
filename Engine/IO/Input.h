#pragma once

#include <bitset>

#include "Math/glm.h"

namespace Button
{
enum button_t {
	Left, Middle, Right, Unknown,
	COUNT
};
}

namespace Key
{
enum key_t {
	Unknown = 0,

	Left, Right, Up, Down,
	Backspace = '\b', Tab = '\t', Return = '\r', Enter = '\r',

	LeftControl, LeftShift, LeftAlt, LeftSystem,
	RightControl, RightShift, RightAlt, RightSystem,
	End, Home, Insert, PageUp, PageDown,

	Escape = '\033', Space = ' ', Exclaim = '!', DoubleQuote = '"',
	Hash = '#', Dollar = '$', Percent = '%', Ampersand = '&',
	Quote = '\'', LeftParen = '(', RightParen = ')',
	Asterisk = '*', Multiply = '*', Plus = '+', Comma = ',', Minus = '-', Period = '.', Slash = '/', Divide = '/',

	Num0 = '0', Num1 = '1', Num2 = '2', Num3 = '3', Num4 = '4',
	Num5 = '5', Num6 = '6', Num7 = '7', Num8 = '8', Num9 = '9',

	Colon = ':', Semicolon = ';', Less = '<', Equal = '=', Greater = '>', Question = '?', At = '@',

	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,

	LeftBracket = '[', Backslash = '\\', RightBracket = ']',
	Caret = '^', Underscore = '_', Backquote = '`',

	A = 'a', B = 'b', C = 'c', D = 'd', E = 'e', F = 'f', G = 'g',
	H = 'h', I = 'i', J = 'j', K = 'k', L = 'l', M = 'm', N = 'n',
	O = 'o', P = 'p', Q = 'q', R = 'r', S = 's', T = 't', U = 'u',
	V = 'v', W = 'w', X = 'x', Y = 'y', Z = 'z',

	LeftBrace = '{', Pipe = '|', RightBrace = '}', Tilde = '~',

	Delete = '\177',

	COUNT
};
}

struct Input
{
	// Window
	static struct SDL_Window *window() { return win; }
	static void close_window();

	static bool  window_closed() { return closed; }
	static bool  window_focused() { return has_focus; }
	static ivec2 window_size() { return dim; }
	static void  set_window_size(ivec2 _size);

	// Keyboard
	static inline bool key_down(Key::key_t _key);
	static inline bool key_changed(Key::key_t _key);
	static inline bool key_pressed (Key::key_t _key);
	static inline bool key_released(Key::key_t _key);
	static inline bool *keys_down() { return keyboard_state[keyboard_index]; }

	static inline const char *input_chars() { return text_input; }

	// Mouse
	static inline bool button_down(Button::button_t _button);
	static inline bool button_changed(Button::button_t _button);
	static inline bool button_pressed(Button::button_t _button);
	static inline bool button_released(Button::button_t _button);

	static int  wheel_scroll() { return mouse_wheel_delta; }
	static ivec2 mouse_delta() { return mouse_pos_delta; }
	static vec2 mouse_delta_ss() { return vec2((float)mouse_pos_delta.x / dim.x, -(float)mouse_pos_delta.y / dim.y); }


	static ivec2 mouse_position(bool openGLSpace = true) { return ivec2(mouse_pos.x, openGLSpace ? dim.y - mouse_pos.y : mouse_pos.y); }
	static vec2 mouse_position_ss();
	static void set_mouse_position(ivec2 _pos, bool openGLSpace = true);

private:
	/// Methods (private)
	static void init(struct SDL_Window *window);
	static void poll_events();

	/// Attributes (private)
	static struct SDL_Window *win;

	static ivec2 dim, center;

	static ivec2 prev_mouse_pos, mouse_pos;
	static ivec2 mouse_pos_delta;
	static int mouse_wheel_delta;

	static bool has_focus;
	static bool closed;

	static int mouse_index, keyboard_index;
	static bool mouse_cleared, keyboard_cleared;

	static bool mouse_state[2][Button::COUNT];
	static bool keyboard_state[2][Key::COUNT];

	static char text_input[];

	friend struct Engine;
};

// Keyboard
bool Input::key_down(Key::key_t _key)
{
	return keyboard_state[keyboard_index][_key];
}

bool Input::key_changed(Key::key_t _key)
{
	return (keyboard_state[0][_key] != keyboard_state[1][_key]);
}

bool Input::key_pressed(Key::key_t _key)
{
	return key_changed(_key) & key_down(_key);
}

bool Input::key_released(Key::key_t _key)
{
	return key_changed(_key) & !key_down(_key);
}



// Mouse
bool Input::button_down(Button::button_t _button)
{
	return mouse_state[mouse_index][_button];
}

bool Input::button_changed(Button::button_t _button)
{
	return (mouse_state[0][_button] != mouse_state[1][_button]);
}

bool Input::button_pressed(Button::button_t _button)
{
	return button_changed(_button) & button_down(_button);
}

bool Input::button_released(Button::button_t _button)
{
	return button_changed(_button) & !button_down(_button);
}
