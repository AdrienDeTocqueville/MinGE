#ifndef BALLSCRIPT_H_INCLUDED
#define BALLSCRIPT_H_INCLUDED

#include <MinGE.h>

class BallScript : public Script
{
    public:
        static void loadSound()
        {
            buffer.loadFromFile("Resources/choc.wav");
            sound.setBuffer(buffer);
        }

        void onCollision(const Collision& _collision) override
        {
            if (_collision.entity->getTagName() == "Ball")
            {
                float intensity = dot(_collision.relativeVelocity, _collision.normal);
                sound.setVolume( clamp(abs(intensity)*40.0f, 0.0f, 100.0f) );

                vec3 p = _collision.point;
                sound.setPosition(p.x, p.z, p.y);

                sound.play();
            }
        }

        static sf::SoundBuffer buffer;
        static sf::Sound sound;
};

#endif // BALLSCRIPT_H_INCLUDED
