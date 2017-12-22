#include "Physic/CollisionDetection.h"
#include "Physic/ContactConstraint.h"

#include "Components/RigidBody.h"
#include "Components/Collider.h"
#include "Components/Transform.h"

//#include "Utility/Input.h"

#include "Components/Sphere.h"

#include <set>

namespace MinGE
{

Point support(Collider* a, Collider* b, vec3 _axis)
{
    vec3 supA = a->getSupport( _axis);
    vec3 supB = b->getSupport(-_axis);

    return Point(supA, supB);
}

/// GJK
bool GJK(Collider* a, Collider* b, Simplex& _simplex)
{
    vec3 axis = b->find<Transform>()->position - a->find<Transform>()->position;

    // Permet a la fonction de quitter plus rapidement en cas de non intersection
    // 1er tour
        axis = normalize(axis);

        Point supportPoint = support(a, b, axis);
        if (dot(supportPoint.p, axis) < 0)
            return false;

        _simplex.push_back(supportPoint);

    // 2eme tour
        axis = normalize(-supportPoint.p);

        supportPoint = support(a, b, axis);
        if (dot(supportPoint.p, axis) < 0)
            return false;

        _simplex.push_back(supportPoint);

        // Determine le nouvel axe de recherche
        vec3 AB = _simplex[0].p-_simplex[1].p;
        axis = cross(cross(AB, -_simplex[1].p), AB);   // A gauche ou a droite de la ligne (-_simplex[1] = AO)



    unsigned steps = 0;

    while (steps++ < 50)  // Evite les boucles infinies
    {
        axis = normalize(axis);

        supportPoint = support(a, b, axis);
        if (dot(supportPoint.p, axis) <= 0)
            return false;

        _simplex.push_back(supportPoint);

        if (containsOrigin(_simplex, axis))
            return true;
    }

    return false;
}

bool containsOrigin(Simplex& _simplex, vec3& _axis)
{
    // A est le dernier point ajouté

    if (_simplex.size() == 4)
    {
        vec3 AO = -_simplex[3].p;

        vec3 B = _simplex[2].p, C = _simplex[1].p, D = _simplex[0].p;
        vec3 AB(B + AO), AC(C + AO), AD(D + AO);

        //On regarde si l'origine se trouve a l'exterieur du tetrahedre
        //en la projetant sur les plans formés palair les faces
        //Si elle est 'devant' une face, on effectue un test avec le triangle (checkTriangle)

        vec3 ABC = cross(AB, AC); // Vecteur normal au triangle ABC

        if (dot(ABC, AO) >= 0.0f)   // Devant ABC
        {
            _simplex.erase(_simplex.begin() + 0); // On enleve D
            return checkTriangle(ABC, AO, AB, AC, _simplex, _axis);
        }


        vec3 ADB = cross(AD, AB); // Vecteur normal au triangle ADB

        if (dot(ADB, AO) >= 0.0f)   // Devant ADB
        {
            _simplex.erase(_simplex.begin() + 1); // On enleve C
            return checkTriangle(ADB, AO, AD, AB, _simplex, _axis);
        }


        vec3 ACD = cross(AC, AD); // Vecteur normal au triangle ACD

        if (dot(ACD, AO) >= 0.0f)   // Devant ACD
        {
            _simplex.erase(_simplex.begin() + 2); // On enleve B
            return checkTriangle(ACD, AO, AC, AD, _simplex, _axis);
        }


        return true;    // l'origine est a l'interieur du tetrahedre
    }
    else
    {
        vec3 AO = -_simplex[2].p;

        vec3 B = _simplex[1].p, C = _simplex[0].p;
        vec3 AB(B + AO), AC(C + AO);

        vec3 ABC = cross(AB, AC); // Vecteur normal au triangle

        if (dot(cross(ABC, AC), AO) >= 0.0f)    // Du cote AC
        {
            _simplex.erase(_simplex.begin() + 1); // On enleve B
            _axis = cross(cross(AC, AO), AC);

            return false;
        }

        if (dot(cross(AB, ABC), AO) >= 0.0f)    // Du cote AB
        {
            _simplex.erase(_simplex.begin() + 0); // On enleve C
            _axis = cross(cross(AB, AO), AB);

            return false;
        }

        if (dot(ABC, AO) > 0)   // Au dessus du triangle
            _axis = ABC;
        else                    // Au dessous du triangle
        {
            std::swap(_simplex[0], _simplex[1]); // on inverse B et C

            _axis = -ABC;
        }

        return false;
    }

    // la ligne n'est pas possible grace au test initial
}

bool checkTriangle(vec3& ABC, vec3& AO, vec3& AB, vec3& AC, Simplex& _simplex, vec3& _axis)
{
    if (dot(cross(ABC, AC), AO) >= 0.0f)    // Du cote AC
    {
        _simplex.erase(_simplex.begin() + 1); // On enleve B
        _axis = cross(cross(AC, AO), AC);

        return false;
    }

    if (dot(cross(AB, ABC), AO) >= 0.0f)    // Du cote AB
    {
        _simplex.erase(_simplex.begin() + 0); // On enleve C
        _axis = cross(cross(AB, AO), AB);

        return false;
    }

    // Sinon, il est au dessus du triangle
    _axis = ABC;

    return false;
}


/// EPA
struct Closer
{
    bool operator() (const Face& a, const Face& b) const
    { return a.distance < b.distance; }
};

Manifold* EPA(Collider* _a, Collider* _b, Simplex& _simplex)
{
    static unsigned maxSteps = 20;

//    if (Input::getKeyReleased(sf::Keyboard::T))
//        maxSteps--;
//    if (Input::getKeyReleased(sf::Keyboard::Y))
//        maxSteps++;
//
//    maxSteps = min(maxSteps, 20);

    Face::simplex = &_simplex;

    std::multiset<Face, Closer> faces;
    std::multiset<Face, Closer>::iterator closest;
        faces.insert(Face(3, 2, 1)); // ABC
        faces.insert(Face(3, 1, 0)); // ACD
        faces.insert(Face(3, 0, 2)); // ADB
        faces.insert(Face(2, 0, 1)); // BDC


    unsigned steps = 0;
    float distance;

    while (steps++ < maxSteps)
    {
        closest = faces.begin();

        /// Recherche du point de support
        Point supportPoint = support(_a, _b, closest->normal);
        distance = dot(supportPoint.p, closest->normal);

        /// Test de la condition de terminaison
        if (epsilonEqual(distance, closest->distance, EPSILON))  // On ne peut plus agrandir le polytope: on a la solution
            break;

        /// Agrandissement du polytope
        std::vector<Edge> edges;

        auto faceIt = faces.begin();
        while(faceIt != faces.end())
        {
            // Si le produit scalaire est positif, le polytope sera concave
            if (dot(faceIt->normal, supportPoint.p - _simplex[faceIt->a].p) >= 0.0f)
            {
                // On garde donc les bords
                // Si il y en a un en double, on le retire de la liste
                addEdge(edges, Edge(faceIt->a, faceIt->b));
                addEdge(edges, Edge(faceIt->b, faceIt->c));
                addEdge(edges, Edge(faceIt->c, faceIt->a));

                // On supprime cette face de la liste
                faceIt = faces.erase(faceIt);
            }
            else
                ++faceIt;
        }


        // Construction des nouvelles faces
        _simplex.push_back(supportPoint);
        unsigned spIndex = _simplex.size() -1;

        for (auto& edge: edges)
            faces.insert(Face(spIndex, edge.first, edge.second));
    }

    /// Genere les informations de contact
    vec3 lambdas; // Coordonnées barycentriques du projeté de l'origine sur la face la plus proche
    getBarycentricCoordinates(distance * closest->normal, *closest, lambdas);

    vec3 A1 = _simplex[closest->a].supA, A2 = _simplex[closest->b].supA, A3 = _simplex[closest->c].supA;
    vec3 B1 = _simplex[closest->a].supB, B2 = _simplex[closest->b].supB, B3 = _simplex[closest->c].supB;

    Manifold* man = new Manifold();
        man->points[0] = lambdas.x*A1 + lambdas.y*A2 + lambdas.z*A3;
        man->points[1] = lambdas.x*B1 + lambdas.y*B2 + lambdas.z*B3;

        man->normal = closest->normal;
        man->penetration = -distance;
        computeBasis(man->normal, man->u, man->v);

    return man;
}

Simplex* Face::simplex;

Face::Face(unsigned _a, unsigned _b, unsigned _c):
    a(_a), b(_b), c(_c)
{
    vec3 AB = (*simplex)[b].p - (*simplex)[a].p;
    vec3 AC = (*simplex)[c].p - (*simplex)[a].p;

    normal = normalize(cross(AB, AC));
    distance = dot((*simplex)[a].p, normal); // N'importe quel point fonctionne (a, b ou c)
}

void Face::draw() const
{
    Debug::drawLine((*simplex)[a].p, (*simplex)[b].p, vec3(0.0f));
    Debug::drawLine((*simplex)[a].p, (*simplex)[c].p, vec3(0.0f));
    Debug::drawLine((*simplex)[b].p, (*simplex)[c].p, vec3(0.0f));

    // Normal
//    vec3 center = (*simplex)[a].p + (*simplex)[b].p + (*simplex)[c].p;
//    Debug::drawVector(center/3.0f, normal, vec3(0.7f, 0.2f, 0.2f));
}

void addEdge(std::vector<Edge>& edges, Edge b)
{
    for (unsigned i(0) ; i < edges.size() ; i++)
    {
        if (edges[i].first == b.second && edges[i].second == b.first)
        {
            edges.erase(edges.begin() + i);
            return;
        }
    }

    edges.push_back(b);
}

void getBarycentricCoordinates(const vec3& point, const Face& triangle, vec3& lambdas)
{
    vec3 AB = (*Face::simplex)[triangle.b].p - (*Face::simplex)[triangle.a].p;
    vec3 AC = (*Face::simplex)[triangle.c].p - (*Face::simplex)[triangle.a].p;
    vec3 AP = point - (*Face::simplex)[triangle.a].p;

    float ab = dot(AB, AB);
    float ac = dot(AC, AC);
    float ABdAC = dot(AB, AC);
    float ABdAP = dot(AB, AP);
    float ACdAP = dot(AC, AP);

    float scalar = 1.0f / (ab*ac - ABdAC*ABdAC);
    lambdas.y = (ac*ABdAP - ABdAC*ACdAP) * scalar;
    lambdas.z = (ab*ACdAP - ABdAC*ABdAP) * scalar;
    lambdas.x = 1.0f - lambdas.y - lambdas.z;
}

// http://box2d.org/2014/02/computing-a-basis/
void computeBasis(const vec3& a, vec3& b, vec3& c)
{
  // Suppose vector a has all equal components and is a unit vector: a = (s, s, s)
  // Then 3*s*s = 1, s = sqrt(1/3) = 0.57735. This means that at least one component of a
  // unit vector must be greater or equal to 0.57735.

  if (abs(a.x) >= 0.57735f)
    b = vec3(a.y, -a.x, 0.0f);
  else
    b = vec3(0.0f, a.z, -a.y);

  b = normalize(b);
  c = cross(a, b);
}

/// Custom
Manifold* SphereSphere(Collider* _a, Collider* _b)
{
    Sphere* a = reinterpret_cast<Sphere*>(_a);
    Sphere* b = reinterpret_cast<Sphere*>(_b);

    vec3 normal = b->find<Transform>()->position - a->find<Transform>()->position;

    float radiusSum = a->getRadius() + b->getRadius();

    float squaredDistance = length2(normal);
    if (epsilonEqual(squaredDistance, 0.0f, EPSILON*EPSILON))
    {
        Manifold* man = new Manifold();
            man->normal = vec3(0, 0, 1);
        man->penetration = -radiusSum;

        man->points[0] = a->find<Transform>()->position + a->getRadius() * man->normal;
        man->points[1] = b->find<Transform>()->position - b->getRadius() * man->normal;

        return man;
    }

    if (squaredDistance >= radiusSum*radiusSum)
        return nullptr;

    float distance = sqrt(squaredDistance);


    Manifold* man = new Manifold();
        man->normal = normal/distance;
        man->penetration = -radiusSum + distance;

        man->points[0] = a->find<Transform>()->position + a->getRadius() * man->normal;
        man->points[1] = b->find<Transform>()->position - b->getRadius() * man->normal;

    return man;
}

}
