#include "Input.h"

sf::RenderWindow* Input::window = nullptr;
sf::Event* Input::event = nullptr;

vec2 Input::dim(0.0f), Input::center(0.0f);

vec2 Input::prevMousePos(0.0f), Input::mousePos(0.0f);
vec2 Input::delta(0.0f);
int Input::wheelDelta(0);

CursorMode Input::mode = GE_FREE;

bool Input::focus = true;
bool Input::closed = false;

int Input::mouseIndex = 0, Input::keyboardIndex = 0;
bool Input::mouseNeedsUpdate = true, Input::keyboardNeedsUpdate = true;

std::bitset<sf::Mouse::ButtonCount> Input::mouseState[2];
std::bitset<sf::Keyboard::KeyCount> Input::keyboardState[2];

sf::Uint32 Input::unicode;

/// Methods (private)
void Input::init(sf::RenderWindow* _window)
{
	window = _window;
	event = new sf::Event();

	dim = toVec2(window->getSize());
	center = ivec2(0.5f*dim);

	prevMousePos = center;
	mousePos = center;

	focus = true;
	closed = false;
}

void Input::destroy()
{
	delete event;

	window = nullptr;
	event = nullptr;

	focus = true;
	closed = true;
}

void Input::update()
{
	bool mouseEvent = false, keyboardEvent = false;

	wheelDelta = 0;
	unicode = 0;

	while (window->pollEvent(*event))
	{
		switch (event->type)
		{
			case sf::Event::Closed:
				closed = true;
			break;

			case sf::Event::MouseMoved:
				mousePos = toVec2(sf::Mouse::getPosition(*window));
			break;

			case sf::Event::MouseWheelMoved:
				wheelDelta = event->mouseWheel.delta;
			break;

			case sf::Event::LostFocus:
				focus = false;
			break;

			case sf::Event::GainedFocus:
				focus = true;
			break;

			case sf::Event::Resized:
				dim = vec2(event->size.width, event->size.height);
				center = ivec2(0.5f*dim);

				if (mode == GE_CAPTURE)
					prevMousePos = center;
			break;

			case sf::Event::TextEntered:
				unicode = event->text.unicode;
			break;

			case sf::Event::MouseButtonPressed:
			case sf::Event::MouseButtonReleased:
				mouseEvent = true;
				mouseNeedsUpdate = true;
			break;

			case sf::Event::KeyPressed:
			case sf::Event::KeyReleased:
				keyboardEvent = true;
				keyboardNeedsUpdate = true;
			break;

			default: break;
		}
	}

	/// Mouse click
	if (mouseEvent)
	{
		for (int i(0) ; i < sf::Mouse::ButtonCount ; i++)
			mouseState[mouseIndex][i] = sf::Mouse::isButtonPressed( (sf::Mouse::Button)i );
	}
	else if (mouseNeedsUpdate)
	{
		mouseNeedsUpdate = false;
		mouseState[mouseIndex] = mouseState[1-mouseIndex];
	}

	if (mouseNeedsUpdate)
		mouseIndex = 1-mouseIndex;

	/// Keyboard press
	if (keyboardEvent)
	{
		for (int i(0) ; i < sf::Keyboard::KeyCount ; i++)
			keyboardState[keyboardIndex][i] = sf::Keyboard::isKeyPressed( (sf::Keyboard::Key)i );
	}
	else if (keyboardNeedsUpdate)
	{
		keyboardNeedsUpdate = false;
		keyboardState[keyboardIndex] = keyboardState[1-keyboardIndex];
	}

	if (keyboardNeedsUpdate)
		keyboardIndex = 1-keyboardIndex;


	/// Mouse move
	delta = mousePos - prevMousePos;
	delta.x *= -1;


	if (mode == GE_CAPTURE)
		sf::Mouse::setPosition(toSFVec2i(center), *window);
	else
		prevMousePos = mousePos;
}

/// Methods (private)
void Input::setWindowSize(vec2 _size)
{
	window->setSize(toSFVec2u(_size));

	dim = _size;
	center = ivec2(0.5f*dim);

	if (mode == GE_CAPTURE)
		prevMousePos = center;
}

/// Methods (public)
bool Input::hasFocus()
{
	return focus;
}

bool Input::isOpen()
{
	return !closed;
}

bool Input::textIsChar()
{
	return (unicode >= 32 && unicode < 128);
}

bool Input::textIsNum()
{
	return (unicode >= '0' && unicode <= '9');
}

void Input::close()
{
	window->close();
}

/// Setter
void Input::setCursorMode(CursorMode _mode)
{
	if (mode == _mode)
		return;

	mode = _mode;

	sf::Mouse::setPosition(toSFVec2i(center), *window);
	delta = vec2(0.0f);

	if (mode == GE_CAPTURE)
		prevMousePos = center;

	if (mode == GE_FREE_HIDDEN || mode == GE_CAPTURE)
		window->setMouseCursorVisible(false);
	else
		window->setMouseCursorVisible(true);
}

void Input::setMousePosition(vec2 _pos)
{
	sf::Mouse::setPosition(toSFVec2i(_pos), *window);
}

/// Getters
CursorMode Input::getCursorMode()
{
	return mode;
}

// Window
sf::Event* Input::getEvent()
{
	return event;
}

sf::RenderWindow* Input::getWindow()
{
	return window;
}

vec2 Input::getWindowSize()
{
	return dim;
}

// Keyboard
bool Input::getKeyDown(sf::Keyboard::Key _key)
{
	return keyboardState[keyboardIndex][_key];
}

bool Input::getKeyPressed(sf::Keyboard::Key _key)
{
	if (keyboardState[keyboardIndex][_key] == keyboardState[1-keyboardIndex][_key])
		return false;

	return keyboardState[keyboardIndex][_key];
}

bool Input::getKeyReleased(sf::Keyboard::Key _key)
{
	if (keyboardState[keyboardIndex][_key] == keyboardState[1-keyboardIndex][_key])
		return false;

	return !keyboardState[keyboardIndex][_key];
}

char Input::getText()
{
	return static_cast<char>(unicode);
}

// Mouse
bool Input::getMouseDown(sf::Mouse::Button _button)
{
	return mouseState[mouseIndex][_button];
}

bool Input::getMousePressed(sf::Mouse::Button _button)
{
	if (mouseState[mouseIndex][_button] == mouseState[1-mouseIndex][_button])
		return false;

	return mouseState[mouseIndex][_button];
}

bool Input::getMouseReleased(sf::Mouse::Button _button)
{
	if (mouseState[mouseIndex][_button] == mouseState[1-mouseIndex][_button])
		return false;

	return !mouseState[mouseIndex][_button];
}

vec2 Input::getMousePosition(bool openGLSpace)
{
	if (openGLSpace)
		return vec2(mousePos.x, window->getSize().y - mousePos.y);

	return mousePos;
}

vec2 Input::getMousePositionRelative()
{
	vec2 mousePosRel(mousePos);

	mousePosRel.x /= window->getSize().x;
	mousePosRel.y = 1.0f - mousePosRel.y/window->getSize().y;

	return mousePosRel*2.0f - 1.0f;
}

vec2 Input::getMouseDelta()
{
	return delta;
}

vec2 Input::getMouseDeltaRelative()
{
	vec2 deltaRel(delta);

	deltaRel.x /= window->getSize().x;
	deltaRel.y = 1.0f - deltaRel.y/window->getSize().y;

	return deltaRel*2.0f - 1.0f;
}

int Input::getMouseWheelDelta()
{
	return wheelDelta;
}
