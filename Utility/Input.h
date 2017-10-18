#ifndef INPUT_H
#define INPUT_H

#include "includes.h"

enum CursorMode {GE_FREE, GE_CAPTURE};

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
            static vec2 getMouseDelta();
            static int getMouseWheelDelta();

            static sf::Event* getEvent();
            static sf::RenderWindow* getWindow();

            static vec2 getWindowSize();

            static bool getKeyDown(sf::Keyboard::Key _key);
            static bool getKeyPressed (sf::Keyboard::Key _key);
            static bool getKeyReleased(sf::Keyboard::Key _key);

            static vec2 getMousePosition(bool openGLSpace = true);

            static bool getMouseDown(sf::Mouse::Button _button);
            static bool getMousePressed(sf::Mouse::Button _button);
            static bool getMouseReleased(sf::Mouse::Button _button);

            static char getText();

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

            static vec2 delta;
            static int wheelDelta;

            static CursorMode mode;

            static bool focus;
            static bool close;

            static std::array<bool, sf::Mouse::ButtonCount>  mouseStatePrev;
            static std::array<bool, sf::Mouse::ButtonCount>  mouseState;

            static std::array<bool, sf::Keyboard::KeyCount>  keyboardStatePrev;
            static std::array<bool, sf::Keyboard::KeyCount>  keyboardState;
};

#endif // INPUT_H
