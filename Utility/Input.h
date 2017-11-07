#pragma once

#include "includes.h"

enum CursorMode {GE_FREE, GE_FREE_HIDDEN, GE_CAPTURE};

class Input
{
    friend class Engine;

    public:
        /// Methods (public)
            static bool hasFocus();
            static bool isClosed();

            static bool textIsChar();
            static bool textIsNum();

        /// Setters
            static void setCursorMode(CursorMode _mode);
            static void setMousePosition(vec2 _pos);

        /// Getters
            static CursorMode getCursorMode();

            // Window
            static sf::Event* getEvent();
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
            static sf::Event* event;

            static vec2 dim, center;

            static vec2 prevMousePos, mousePos;
            static vec2 delta;
            static int wheelDelta;

            static CursorMode mode;

            static bool focus;
            static bool close;

            static int stateIndex;
            static bool mouseState[2][sf::Mouse::ButtonCount];
            static bool keyboardState[2][sf::Keyboard::KeyCount];
};
