#ifndef SHAPE_H
#define SHAPE_H

#include "includes.h"

struct AABB
{
    void prepare();
    static bool collide(AABB* a, AABB* b);

    static void draw();

    vec3 center = vec3(0.0f);
    vec3 dim = vec3(0.0f);

    vec3 color;

    static unsigned vbo;
    static unsigned vao;

    static std::vector<vec3> vertices;
    static std::vector<vec3> colors;

    static bool drawAABBs;
};

#endif // SHAPE_H
