#ifndef INCLUDES_H_INCLUDED
#define INCLUDES_H_INCLUDED

#include <GL/glew.h>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Audio.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <cmath>
#include <algorithm>

#include <functional>
#include <vector>
#include <array>
#include <list>
#include <map>
#include <set>

#include <glm.hpp>
#include <gtc/epsilon.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>
#include <gtx/transform.hpp>
#include <gtx/matrix_operation.hpp>


#include <Windows.h>


const float PI = 3.14159265358979323846f;
const float EPSILON = 0.0001f;

using namespace glm;

template <typename T>
std::string toString(T _number)
{
    std::stringstream os;
    os << _number;

    return os.str();
}

vec2 toVec2(sf::Vector2i v);
vec2 toVec2(sf::Vector2u v);
sf::Vector2i toSFVec2i(vec2 v);
sf::Vector2u toSFVec2u(vec2 v);

vec3 vecClamp(vec3 v);

void write(vec3 _vec, bool ret = true);

enum ErrorType {WARNING, FILE_NOT_FOUND, OPENGL_ERROR, MINGE_ERROR};

// Implementation in "Util/Errors.cpp"
class Error
{
    public:
        static void init();

        static void add(ErrorType _type, std::string _description);

        static bool check();

    private:
        Error();
        ~Error();

    /// Attributes (private)
        static const unsigned maxType = MINGE_ERROR+1;
        static bool error;

    /// Methods (private)
        static std::string getTitle(ErrorType _type);
        static int getIcon(ErrorType _type);
};

#endif // INCLUDES_H_INCLUDED
