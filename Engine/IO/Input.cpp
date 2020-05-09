#include "IO/Input.h"

#include "Profiler/profiler.h"

#include <SFML/System.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

sf::RenderWindow* Input::win = nullptr;

ivec2 Input::dim(0), Input::center(0);

vec2 Input::prev_mouse_pos(0.0f), Input::mouse_pos(0.0f);
vec2 Input::mouse_pos_delta(0.0f);
int Input::mouse_wheel_delta(0);

Input::Cursor Input::mode = Input::Cursor::Free;

bool Input::has_focus, Input::closed;

int Input::mouse_index = 0, Input::keyboard_index = 0;
bool Input::mouse_cleared = false, Input::keyboardCleared = false;

std::bitset<sf::Mouse::ButtonCount> Input::mouse_state[2];
std::bitset<sf::Keyboard::KeyCount> Input::keyboard_state[2];


static inline vec2 toVec2(sf::Vector2i v)	{ return vec2(v.x, v.y); }
static inline vec2 toVec2(sf::Vector2u v)	{ return vec2(v.x, v.y); }
static inline sf::Vector2i toSFVec2i(vec2 v)	{ return sf::Vector2i((int)v.x, (int)v.y); }
static inline sf::Vector2i toSFVec2i(ivec2 v)	{ return sf::Vector2i(v.x, v.y); }
static inline sf::Vector2u toSFVec2u(ivec2 v)	{ return sf::Vector2u(v.x, v.y); }


/// Methods (private)
void Input::init(sf::RenderWindow* _window)
{
	win = _window;
	win->setKeyRepeatEnabled(false);

	dim = toVec2(win->getSize());
	center = ivec2(dim / 2);

	prev_mouse_pos = center;
	mouse_pos = center;

	has_focus = true;
	closed = false;
}

void Input::poll_events()
{
	MICROPROFILE_SCOPEI("IO_INPUT", "update");

	bool mouseEvent = false, keyboardEvent = false;

	mouse_wheel_delta = 0;

	sf::Event event;
	while (win->pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			closed = true;
			break;

		case sf::Event::MouseMoved:
			mouse_pos = toVec2(sf::Mouse::getPosition(*win));
			break;

		case sf::Event::MouseWheelScrolled:
			mouse_wheel_delta = (int)event.mouseWheelScroll.delta;
			break;

		case sf::Event::LostFocus:
			has_focus = false;
			break;

		case sf::Event::GainedFocus:
			has_focus = true;
			break;

		case sf::Event::Resized:
			dim = ivec2(event.size.width, event.size.height);
			center = ivec2(dim / 2);

			if (mode == Cursor::Capture)
				prev_mouse_pos = center;
			break;

		case sf::Event::MouseButtonPressed:
		case sf::Event::MouseButtonReleased:
			if (!mouseEvent) {
				mouseEvent = true;
				mouse_cleared = false;
				mouse_index = 1-mouse_index;
				mouse_state[mouse_index] = mouse_state[1-mouse_index];
			}
			mouse_state[mouse_index][event.mouseButton.button] = (event.type == sf::Event::MouseButtonPressed);
			break;

		case sf::Event::KeyPressed:
		case sf::Event::KeyReleased:
			if (event.key.code == sf::Keyboard::Unknown)
				break;
			if (!keyboardEvent) {
				keyboardEvent = true;
				keyboardCleared = false;
				keyboard_index = 1-keyboard_index;
				keyboard_state[keyboard_index] = keyboard_state[1-keyboard_index];
			}
			keyboard_state[keyboard_index][event.key.code] = (event.type == sf::Event::KeyPressed);
			break;

		default: break;
		}
	}

	if (!mouse_cleared && !mouseEvent)
	{
		mouse_cleared = true;
		mouse_state[1-mouse_index] = mouse_state[mouse_index];
	}
	if (!keyboardCleared && !keyboardEvent)
	{
		keyboardCleared = true;
		keyboard_state[1-keyboard_index] = keyboard_state[keyboard_index];
	}


	/// Mouse move
	mouse_pos_delta = mouse_pos - prev_mouse_pos;
	mouse_pos_delta.x *= -1;


	if (mode == Cursor::Capture)
		sf::Mouse::setPosition(toSFVec2i(center), *win);
	else
		prev_mouse_pos = mouse_pos;
}


// Window
void Input::close_window()
{
	win->close();
}

void Input::set_window_size(ivec2 _size)
{
	win->setSize(toSFVec2u(_size));

	dim = _size;
	center = ivec2(dim / 2);

	if (mode == Cursor::Capture)
		prev_mouse_pos = center;
}


// Mouse
vec2 Input::mouse_delta_ss()
{
	vec2 deltaRel(mouse_pos_delta);

	deltaRel.x /= win->getSize().x;
	deltaRel.y = 1.0f - deltaRel.y/win->getSize().y;

	return deltaRel*2.0f - 1.0f;
}

vec2 Input::mouse_position(bool openGLSpace)
{
	if (openGLSpace)
		return vec2(mouse_pos.x, win->getSize().y - mouse_pos.y);

	return mouse_pos;
}

vec2 Input::mouse_position_relative()
{
	vec2 mousePosRel(mouse_pos);

	mousePosRel.x /= win->getSize().x;
	mousePosRel.y = 1.0f - mousePosRel.y/win->getSize().y;

	return mousePosRel*2.0f - 1.0f;
}

void Input::set_mouse_position(vec2 _pos, bool openGLSpace)
{
	if (openGLSpace)
		_pos.y = win->getSize().y - _pos.y;

	sf::Mouse::setPosition(toSFVec2i(_pos), *win);
}

void Input::set_cursor_mode(Input::Cursor _mode)
{
	if (mode == _mode)
		return;

	mode = _mode;

	sf::Mouse::setPosition(toSFVec2i(center), *win);
	mouse_pos_delta = vec2(0.0f);

	if (mode == Cursor::Capture)
		prev_mouse_pos = center;

	if (mode == Cursor::FreeHidden || mode == Cursor::Capture)
		win->setMouseCursorVisible(false);
	else
		win->setMouseCursorVisible(true);
}
