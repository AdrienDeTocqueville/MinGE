#include "IO/Input.h"

#include "Profiler/profiler.h"

#include <SDL2/SDL.h>

SDL_Window *Input::win = nullptr;

ivec2 Input::dim(0), Input::center(0);

ivec2 Input::prev_mouse_pos(0), Input::mouse_pos(0);
ivec2 Input::mouse_pos_delta(0);
int Input::mouse_wheel_delta(0);

bool Input::has_focus, Input::closed;

int Input::mouse_index = 0, Input::keyboard_index = 0;
bool Input::mouse_cleared = false, Input::keyboard_cleared = false;

bool Input::mouse_state[2][Button::COUNT];
bool Input::keyboard_state[2][Key::COUNT];

static const int scancode_map[256] = {
	Key::Unknown, Key::Unknown, Key::Unknown, Key::Unknown,

	Key::A, Key::B, Key::C, Key::D, Key::E, Key::F, Key::G,
	Key::H, Key::I, Key::J, Key::K, Key::L, Key::M, Key::N,
	Key::O, Key::P, Key::Q, Key::R, Key::S, Key::T, Key::U,
	Key::V, Key::W, Key::X, Key::Y, Key::Z,

	Key::Num1, Key::Num2, Key::Num3, Key::Num4, Key::Num5,
	Key::Num6, Key::Num7, Key::Num8, Key::Num9, Key::Num0,

	Key::Return, Key::Escape, Key::Backspace, Key::Tab, Key::Space,
	Key::Minus, Key::Equal, Key::LeftBracket, Key::RightBracket, Key::Backslash,
	Key::Unknown,
	Key::Semicolon, Key::Quote, Key::Backquote,
	Key::Comma, Key::Period, Key::Slash,
	Key::Unknown,

	Key::F1, Key::F2, Key::F3, Key::F4, Key::F5, Key::F6,
	Key::F7, Key::F8, Key::F9, Key::F10, Key::F11, Key::F12,

	Key::Unknown, Key::Unknown, Key::Unknown, Key::Unknown,

	Key::Home, Key::PageUp, Key::Delete, Key::End, Key::PageDown,
	Key::Right, Key::Left, Key::Down, Key::Up,

	Key::Unknown,

	Key::Multiply, Key::Divide, Key::Minus, Key::Plus, Key::Enter,
	Key::Numpad1, Key::Numpad2, Key::Numpad3, Key::Numpad4, Key::Numpad5,
	Key::Numpad6, Key::Numpad7, Key::Numpad8, Key::Numpad9, Key::Numpad0,
	Key::Period,

	// 100
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,

	// 224
	Key::LeftControl, Key::LeftShift, Key::LeftAlt, Key::Unknown,
	Key::RightControl, Key::RightShift, Key::RightAlt, Key::Unknown,
};

static const int button_map[] = {
	Button::Unknown,
	Button::Left, Button::Middle, Button::Right,
	Button::Unknown, Button::Unknown
};

/// Methods (private)
void Input::init(SDL_Window *window)
{
	win = window;

	SDL_GetWindowSize(win, &dim.x, &dim.y);
	center = dim / 2;

	prev_mouse_pos = center;
	mouse_pos = center;

	has_focus = true;
	closed = false;
}

void Input::poll_events()
{
	MICROPROFILE_SCOPEI("IO_INPUT", "poll_events");

	bool mouse_event = false, keyboard_event = false;

	mouse_wheel_delta = 0;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			closed = true;
			break;

		case SDL_MOUSEMOTION:
			mouse_pos = ivec2(event.motion.x, event.motion.y);
			break;

		case SDL_MOUSEWHEEL:
			mouse_wheel_delta = event.wheel.y;
			break;

		case SDL_WINDOWEVENT:
			switch (event.window.event) {
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				has_focus = true;
				break;

			case SDL_WINDOWEVENT_FOCUS_LOST:
				has_focus = false;
				break;

			case SDL_WINDOWEVENT_RESIZED:
				dim = ivec2(event.window.data1, event.window.data2);
				center = dim / 2;
				break;
			}
			break;

		case SDL_KEYUP:
		case SDL_KEYDOWN:
			if (!keyboard_event) {
				keyboard_event = true;
				keyboard_cleared = false;
				keyboard_index = 1-keyboard_index;
				memcpy(keyboard_state + keyboard_index, keyboard_state + (1-keyboard_index), Key::COUNT * sizeof(bool));
			}
			keyboard_state[keyboard_index][event.key.keysym.sym < 128 ?
				event.key.keysym.sym : scancode_map[event.key.keysym.scancode]] = (event.key.state == SDL_PRESSED);
			break;

		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			if (!mouse_event) {
				mouse_event = true;
				mouse_cleared = false;
				mouse_index = 1-mouse_index;
				memcpy(mouse_state + mouse_index, mouse_state + (1-mouse_index), Button::COUNT * sizeof(bool));
			}
			mouse_state[mouse_index][button_map[event.button.button]] = (event.button.state == SDL_PRESSED);
			break;

		default: break;
		}
	}

	if (!mouse_cleared & !mouse_event)
	{
		mouse_cleared = true;
		memcpy(mouse_state + (1 - mouse_index), mouse_state + mouse_index, Button::COUNT * sizeof(bool));
	}
	if (!keyboard_cleared & !keyboard_event)
	{
		keyboard_cleared = true;
		memcpy(keyboard_state + (1 - keyboard_index), keyboard_state + keyboard_index, Key::COUNT * sizeof(bool));
	}

	/// Mouse move
	mouse_pos_delta = mouse_pos - prev_mouse_pos;
	mouse_pos_delta.x *= -1;
	prev_mouse_pos = mouse_pos;
}


// Window
void Input::close_window()
{
	closed = true;
}

void Input::set_window_size(ivec2 _size)
{
	SDL_SetWindowSize(win, _size.x, _size.y);

	dim = _size;
	center = dim / 2;
}


// Mouse
vec2 Input::mouse_position_ss()
{
	vec2 mousePosRel(mouse_pos);

	mousePosRel.x /= dim.x;
	mousePosRel.y = 1.0f - mousePosRel.y/dim.y;

	return mousePosRel*2.0f - 1.0f;
}

void Input::set_mouse_position(ivec2 _pos, bool openGLSpace)
{
	if (openGLSpace)
		_pos.y = dim.y - _pos.y;

	SDL_WarpMouseInWindow(win, _pos.x, _pos.y);
}
