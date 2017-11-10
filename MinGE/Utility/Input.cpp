#include "Input.h"
#include "Components/Component.h"

sf::RenderWindow* Input::window = nullptr;
sf::Event* Input::event = nullptr;

vec2 Input::dim(0.0f), Input::center(0.0f);

vec2 Input::prevMousePos(0.0f), Input::mousePos(0.0f);
vec2 Input::delta(0.0f);
int Input::wheelDelta(0);

CursorMode Input::mode = GE_FREE;

bool Input::focus = true;
bool Input::close = false;

int Input::stateIndex = 0;
bool Input::mouseState[2][sf::Mouse::ButtonCount] = {false};
bool Input::keyboardState[2][sf::Keyboard::KeyCount] = {false};

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
    close = false;
}

void Input::destroy()
{
    delete event;

    window = nullptr;
    event = nullptr;

    focus = true;
    close = true;
}

void Input::update()
{
    wheelDelta = 0;
    unicode = 0;

    while (window->pollEvent(*event))
    {
        switch (event->type)
        {
            case sf::Event::Closed:
                close = true;
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

            default: break;
        }
    }

    for (int i(0) ; i < sf::Mouse::ButtonCount ; i++)
        mouseState[stateIndex][i] = sf::Mouse::isButtonPressed( (sf::Mouse::Button)i );

    for (int i(0) ; i < sf::Keyboard::KeyCount ; i++)
        keyboardState[stateIndex][i] = sf::Keyboard::isKeyPressed( (sf::Keyboard::Key)i );

    stateIndex = 1-stateIndex;


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
bool Input::isClosed()
{
    return close;
}

bool Input::textIsChar()
{
    return (unicode >= 32 && unicode < 128);
}

bool Input::textIsNum()
{
    return (unicode >= '0' && unicode <= '9');
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
    return keyboardState[stateIndex][_key];
}

bool Input::getKeyPressed(sf::Keyboard::Key _key)
{
    if (keyboardState[stateIndex][_key] == keyboardState[1-stateIndex][_key])
        return false;

    return keyboardState[stateIndex][_key];
}

bool Input::getKeyReleased(sf::Keyboard::Key _key)
{
    if (keyboardState[stateIndex][_key] == keyboardState[1-stateIndex][_key])
        return false;

    return !keyboardState[stateIndex][_key];
}

char Input::getText()
{
    return static_cast<char>(unicode);
}

// Mouse
bool Input::getMouseDown(sf::Mouse::Button _button)
{
    return mouseState[stateIndex][_button];
}

bool Input::getMousePressed(sf::Mouse::Button _button)
{
    if (mouseState[stateIndex][_button] == mouseState[1-stateIndex][_button])
        return false;

    return mouseState[stateIndex][_button];
}

bool Input::getMouseReleased(sf::Mouse::Button _button)
{
    if (mouseState[stateIndex][_button] == mouseState[1-stateIndex][_button])
        return false;

    return !mouseState[stateIndex][_button];
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
