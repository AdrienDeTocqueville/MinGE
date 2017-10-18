#include "Input.h"
#include "Components/Component.h"

sf::RenderWindow* Input::window = nullptr;
sf::Event* Input::event = nullptr;

vec2 Input::dim(0.0f), Input::center(0.0f);

vec2 Input::delta(0.0f);
int Input::wheelDelta(0);

CursorMode Input::mode = GE_FREE;

bool Input::focus = true;
bool Input::close = false;

std::array<bool, sf::Mouse::ButtonCount> Input::mouseStatePrev{ {false} };
std::array<bool, sf::Mouse::ButtonCount> Input::mouseState{ {false} };

std::array<bool, sf::Keyboard::KeyCount> Input::keyboardStatePrev{ {false} };
std::array<bool, sf::Keyboard::KeyCount> Input::keyboardState{ {false} };

sf::Uint32 Input::unicode;

/// Methods (private)
void Input::init(sf::RenderWindow* _window)
{
    window = _window;
    event = new sf::Event();

    dim = toVec2(window->getSize());
    center = ivec2(0.5f*dim);

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
            break;

            case sf::Event::TextEntered:
                unicode = event->text.unicode;
            break;

            default: break;
        }
    }

    mouseStatePrev = mouseState;
    for (int i(0) ; i < sf::Mouse::ButtonCount ; i++)
        mouseState[i] = sf::Mouse::isButtonPressed( (sf::Mouse::Button)i );

    keyboardStatePrev = keyboardState;
    for (int i(0) ; i < sf::Keyboard::KeyCount ; i++)
        keyboardState[i] = sf::Keyboard::isKeyPressed( (sf::Keyboard::Key)i );


    delta = center - toVec2(sf::Mouse::getPosition(*window));
    delta.y *= -1;

    if (mode == GE_CAPTURE)
        sf::Mouse::setPosition(toSFVec2i(center), *window);
}

void Input::setWindowSize(vec2 _size)   // private
{
    window->setSize(toSFVec2u(_size));

    dim = _size;
    center = ivec2(0.5f*dim);
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

    if (mode == GE_FREE)
        window->setMouseCursorVisible(true);
    else
        window->setMouseCursorVisible(false);
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

vec2 Input::getMouseDelta()
{
    return delta;
}

int Input::getMouseWheelDelta()
{
    return wheelDelta;
}

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

bool Input::getKeyDown(sf::Keyboard::Key _key)
{
    return keyboardState[_key];
}

bool Input::getKeyPressed(sf::Keyboard::Key _key)
{
    if (keyboardState[_key] == keyboardStatePrev[_key])
        return false;

    return keyboardState[_key];
}

bool Input::getKeyReleased(sf::Keyboard::Key _key)
{
    if (keyboardState[_key] == keyboardStatePrev[_key])
        return false;

    return !keyboardState[_key];
}

vec2 Input::getMousePosition(bool openGLSpace)
{
    if (!openGLSpace)
        return toVec2(sf::Mouse::getPosition(*window));

    vec2 mPos(toVec2(sf::Mouse::getPosition(*window)));
    return vec2(mPos.x, window->getSize().y - mPos.y);
}

bool Input::getMouseDown(sf::Mouse::Button _button)
{
    return mouseState[_button];
}

bool Input::getMousePressed(sf::Mouse::Button _button)
{
    if (mouseState[_button] == mouseStatePrev[_button])
        return false;

    return mouseState[_button];
}

bool Input::getMouseReleased(sf::Mouse::Button _button)
{
    if (mouseState[_button] == mouseStatePrev[_button])
        return false;

    return !mouseState[_button];
}

char Input::getText()
{
    return static_cast<char>(unicode);
}
