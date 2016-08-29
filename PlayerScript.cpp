#include "PlayerScript.h"

#include <gtc/random.hpp>

BillardScript::BillardScript():
    point(0.0f), angleX(0.0f), angleY(0.0f), load(false)
{ }

/// Methods (public)
void BillardScript::update()
{
    if (Input::getKeyDown(sf::Keyboard::O))
    {
        rb->setLinearVelocity(vec3(0.0f));
        rb->setAngularVelocity(vec3(0.0f));
        tr->position = vec3(0.0f);
        tr->rotation = quat(0.0f, vec3(0.0f));
    }

    if (Input::getKeyDown(sf::Keyboard::Left))
        angleX -= deltaTime*2.0f;
    if (Input::getKeyDown(sf::Keyboard::Right))
        angleX += deltaTime*2.0f;

    if (Input::getKeyDown(sf::Keyboard::Down))
        angleY -= deltaTime*2.0f;
    if (Input::getKeyDown(sf::Keyboard::Up))
        angleY += deltaTime*2.0f;


    vec3 direction = Camera::main->getDirection();
    direction.z = 0.0f; direction = normalize(direction);
    float s = sin(angleY), c = cos(angleX);

    Sphere* sp = getComponents<Sphere>()[0];

    point = rb->getCOM() + sp->getRadius()*vec3(s*c, s*sin(angleX), cos(angleY));

    Debug::drawPoint(point, vec3(249, 54, 13)*(1.0f/255.0f));

    if (load)
    {
        float f = (float)timer.getElapsedTime().asSeconds();
        Debug::drawVector(rb->getCOM(), 3.0f*min(f, 4.0f)*direction, vec3(44, 239, 22)*(1.0f/255.0f));
    }
    else
    {
        Debug::drawVector(rb->getCOM(), 3.0f*direction, vec3(44, 239, 22)*(1.0f/255.0f));
    }

    if (load && Input::getKeyReleased(sf::Keyboard::BackSpace))
        load = false;

    if (!load && Input::getKeyReleased(sf::Keyboard::Return))
    {
        load = true;
        timer.restart();
    }
    else if (Input::getKeyReleased(sf::Keyboard::Return))
    {
        load = false;
        float f = (float)timer.getElapsedTime().asMilliseconds();
        rb->applyForce(min(f, 4000.0f)*direction, point);
        timer.restart();
    }
}


GolfScript::GolfScript():
    intensite(10.0f), niveau(0)
{ }

/// Methods (public)
void GolfScript::update()
{
    if (Input::getKeyDown(sf::Keyboard::O))
    {
        rb->setLinearVelocity(vec3(0.0f));
        rb->setAngularVelocity(vec3(0.0f));
        tr->position = vec3(0.0f, 0.0f, niveau*10.0f + 1.0f);
        tr->rotation = quat(vec3(0.0f));
    }

    vec3 directionTir = Camera::main->getDirection();
    directionTir.z = 0.0f;
    directionTir = normalize(directionTir);

    if (Input::getKeyDown(sf::Keyboard::Up))
        intensite += 10000.0f * deltaTime;
    if (Input::getKeyDown(sf::Keyboard::Down))
        intensite -= 10000.0f * deltaTime;
    if (Input::getKeyReleased(sf::Keyboard::Return))
    {
        rb->applyForceToCOM(intensite*directionTir);
        intensite = 10.0f;
    }
    intensite = clamp(intensite,0.0f,50000.0f);
    Debug::drawVector(rb->getCOM(), intensite * directionTir * 0.0003f, vec3(44, 239, 22) * (1.0f/255.0f));

    if (tr->position.z <= niveau*10.0f - 1.0f)
    {
        niveau++;
        rb->setLinearVelocity(vec3(0.0f));
        rb->setAngularVelocity(vec3(0.0f));
        tr->position = vec3(0.0f, 0.0f, niveau*10.0f + 1.0f);
        tr->rotation = quat(vec3(0.0f));
    }
}


PlayerScript::PlayerScript(Entity* _prot, float _speed, float _minSpeed, float _maxSpeed):
    prot(_prot), speed(_speed), minSpeed(_minSpeed), maxSpeed(_maxSpeed),
    dj(nullptr)
{ }

void PlayerScript::update()
{
    float realSpeed = Input::getKeyDown(sf::Keyboard::LShift) ? maxSpeed: speed;
          realSpeed = Input::getKeyDown(sf::Keyboard::LControl) ? minSpeed: realSpeed;

    vec3 direction = Camera::main->getDirection();
    vec3 right = normalize(cross(direction, vec3(0, 0, 1)));

    float force = 50.0f * rb->getMass() * deltaTime * realSpeed;

    if (Input::getKeyDown(sf::Keyboard::Z))     rb->applyForceToCOM( force * direction);
    if (Input::getKeyDown(sf::Keyboard::S))     rb->applyForceToCOM(-force * direction);
    if (Input::getKeyDown(sf::Keyboard::D))     rb->applyForceToCOM( force * right);
    if (Input::getKeyDown(sf::Keyboard::Q))     rb->applyForceToCOM(-force * right);

    if (Input::getKeyDown(sf::Keyboard::Space))
    {
        rb->setLinearVelocity(vec3(0.0f));
        rb->setAngularVelocity(vec3(0.0f));
    }

    if (Input::getKeyDown(sf::Keyboard::O))
    {
        tr->position = vec3(0.0f, 0, 1);
        tr->rotation = quat(vec3(0.0f));
    }

    if (Input::getKeyReleased(sf::Keyboard::B))
        PhysicEngine::get()->removeConstraint(dj);
    if (Input::getKeyReleased(sf::Keyboard::N))
        PhysicEngine::get()->addConstraint(dj);

    if (Input::getKeyReleased(sf::Keyboard::I))
    {
        for (int x(-1) ; x <= 0 ; x++)
        for (int y(0) ; y <= 0 ; y++)
        {
            prot->clone( vec3(1.5f*x, 1.5f*y, -2) );
        }
    }

    for (unsigned i(0) ; i < 4 ; i++)
    {
        if (Input::getKeyReleased(keys[i]))
        {
            Graphic* g = getComponent<Graphic>();
            if (g)  g->setMesh(shapes[i]);

            co[current]->deregisterComponent();
            current = i;
            co[current]->registerComponent();

            break;
        }
    }
}
