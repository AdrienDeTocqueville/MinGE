#pragma once

#include <bitset>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "Utility/helpers.h"

namespace sf
{ class RenderWindow; }

class Input
{
friend class Engine;

public:
	enum Cursor {Free, FreeHidden, Capture};

	/// Methods (public)
	static bool hasFocus();
	static bool isOpen();

	static bool textIsChar();
	static bool textIsNum();

	static void close();

	/// Setters
	static void setCursorMode(Cursor _mode);
	static void setMousePosition(vec2 _pos);

	/// Getters
	static Cursor getCursorMode();

	// Window
	static sf::RenderWindow* getWindow();

	static vec2 getWindowSize();

	// Keyboard
	static bool getKeyDown(sf::Keyboard::Key _key);
	static bool getKeyPressed (sf::Keyboard::Key _key);
	static bool getKeyReleased(sf::Keyboard::Key _key);

	static char getText();

	// Mouse
	static bool getMouseDown(sf::Mouse::Button _button);
	static bool getMousePressed(sf::Mouse::Button _button);
	static bool getMouseReleased(sf::Mouse::Button _button);

	static vec2 getMousePosition(bool openGLSpace = true);
	static vec2 getMousePositionRelative();

	static vec2 getMouseDelta();
	static vec2 getMouseDeltaRelative();

	static int getMouseWheelDelta();

	static sf::Uint32 unicode;

private:
	/// Methods (private)
	static void init(sf::RenderWindow* _window);
	static void destroy();

	static void update();

	static void setWindowSize(vec2 _size);

	/// Attributes (private)
	static sf::RenderWindow* window;

	static vec2 dim, center;

	static vec2 prevMousePos, mousePos;
	static vec2 delta;
	static int wheelDelta;

	static Cursor mode;

	static bool focus;
	static bool closed;

	static int mouseIndex, keyboardIndex;
	static bool mouseCleared, keyboardCleared;

	static std::bitset<sf::Mouse::ButtonCount> mouseState[2];
	static std::bitset<sf::Keyboard::KeyCount> keyboardState[2];
};
