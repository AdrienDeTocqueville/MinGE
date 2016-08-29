#ifndef TPSCAMERASCRIPT_H
#define TPSCAMERASCRIPT_H

#include <MinGE.h>

class TPSCameraScript : public Script
{
    public:
        TPSCameraScript(Transform* _target, float _sensivity, float _distance, vec3 _offset = vec3(0.0f)):
            target(_target), angles(0.0f), clampAngleY(-0.499f*PI, 0.499f*PI),
            sensivity(_sensivity), distance(_distance), offset(_offset)
        { }

        /// Methods (public)
            void start() override
            {
                Input::getMouseDelta();
            }

            void update() override
            {
                Debug::drawVector(vec3(0.0f), vec3(1, 0, 0), vec3(1, 0, 0));
                Debug::drawVector(vec3(0.0f), vec3(0, 1, 0), vec3(0, 1, 0));
                Debug::drawVector(vec3(0.0f), vec3(0, 0, 1), vec3(0, 0, 1));


                angles += radians(Input::getMouseDelta() * sensivity);
                angles.y = clamp(angles.y, clampAngleY.x, clampAngleY.y);

                distance = max(0.01f, distance - 0.2f*Input::getMouseWheelDelta());
            }

            void lateUpdate() override
            {
//                if (Input::getCursorMode() != GE_CAPTURE)
//                    return;
//
//                vec3 dir = tr->getVectorToWorldSpace(vec3(1, 0, 0));
//                vec3 left = tr->getVectorToWorldSpace(vec3(0, 1, 0));
//
//                float speed = (Input::getKeyDown(sf::Keyboard::LShift)?10.0f:1.0f) * deltaTime;
//
//                if (Input::getKeyDown(sf::Keyboard::Z)) tr->position +=  dir * speed;
//                if (Input::getKeyDown(sf::Keyboard::S)) tr->position -=  dir * speed;
//                if (Input::getKeyDown(sf::Keyboard::Q)) tr->position += left * speed;
//                if (Input::getKeyDown(sf::Keyboard::D)) tr->position -= left * speed;
//
//                tr->rotation = quat(vec3(0.0f, angles.y, angles.x));
//
//                tr->toMatrix();


                tr->position = (target->position + offset) - quat(vec3(0.0f, angles.y, angles.x))*vec3(distance, 0.0f, 0.0f);
                tr->lookAt(target->position + offset);

                tr->toMatrix();
            }

    private:
        Transform* target;

        vec2 angles;
        vec2 clampAngleY;

        float sensivity;
        float distance;

        vec3 offset;
};

#endif // TPSCAMERASCRIPT_H
