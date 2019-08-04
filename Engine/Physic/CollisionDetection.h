#pragma once

#include "includes.h"

class Collider;
class Manifold;

namespace MinGE
{

struct Point
{
    Point(vec3 _supA, vec3 _supB):
        p(_supA - _supB), supA(_supA), supB(_supB)
    { }

    vec3 p;

    vec3 supA, supB;
};

Point support(Collider* a, Collider* b, vec3 _axis);

typedef std::vector<Point> Simplex;

/// GJK
bool GJK(Collider* a, Collider* b, Simplex& _simplex);
bool containsOrigin(Simplex& _simplex, vec3& _axis);

bool checkTriangle(vec3& ABC, vec3& AO, vec3& AB, vec3& AC, Simplex& _simplex, vec3& _axis);

/// EPA
Manifold* EPA(Collider* a, Collider* b, Simplex& _simplex);

struct Face
{
    Face(unsigned _a, unsigned _b, unsigned _c);    // CCW

    void draw() const;

    unsigned a, b, c;
    float distance;
    vec3 normal;

    static Simplex* simplex;
};

typedef std::pair<unsigned, unsigned> Edge;
void addEdge(std::vector<Edge>& edges, Edge b);

void getBarycentricCoordinates(const vec3& point, const Face& triangle, vec3& lambdas);

void computeBasis(const vec3& a, vec3& b, vec3& c);

/// Custom
Manifold* SphereSphere(Collider* _a, Collider* _b);

}
