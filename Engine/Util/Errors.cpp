#include "includes.h"


vec2 toVec2(sf::Vector2i v)
{
    return vec2(v.x, v.y);
}
vec2 toVec2(sf::Vector2u v)
{
    return vec2(v.x, v.y);
}
sf::Vector2i toSFVec2i(vec2 v)
{
    return sf::Vector2i(v.x, v.y);
}
sf::Vector2u toSFVec2u(vec2 v)
{
    return sf::Vector2u(v.x, v.y);
}

vec3 vecClamp(vec3 v)
{
    for (unsigned i(0) ; i < 3 ; i++)
        if (epsilonEqual(v[i], 0.0f, EPSILON))
            v[i] = 0.0f;

    return v;
}

void write(vec3 _vec, bool ret)
{
    std::cout << _vec.x << ", " << _vec.y << ", " << _vec.z;
    if (ret) std::cout << std::endl;
}

bool Error::error = false;

Error::Error()
{ }

Error::~Error()
{ }

void Error::init()
{ }

void Error::add(ErrorType _type, std::string _description)
{
    if (_type >= maxType)
        _type = MINGE_ERROR;

    MessageBox(nullptr, _description.c_str(), getTitle(_type).c_str(), getIcon(_type));

    error = true;
}

bool Error::check()
{
    if (!error)
        return false;

    error = false;

    int r = MessageBox(nullptr, "One or more errors occured.\nDo you really want to continue ?",
                       "MinGE: loading error", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);

    return (r == IDNO);
}

std::string Error::getTitle(ErrorType _type)
{
    switch (_type)
    {
        case WARNING:
            return "Warning";
        break;
        case FILE_NOT_FOUND:
            return "File not found";
        break;
        case OPENGL_ERROR:
            return "OpenGL error";
        break;
        default:
            return "MinGE error";
        break;
    }
}

int Error::getIcon(ErrorType _type)
{
    switch (_type)
    {
        case WARNING:
            return MB_ICONWARNING;
        break;
        case FILE_NOT_FOUND:
            return MB_ICONINFORMATION;
        break;
        case OPENGL_ERROR:
            return MB_ICONERROR;
        break;
        default:
            return MB_ICONERROR;
        break;
    }
}
