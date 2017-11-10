#include "Components/AudioListener.h"
#include "Components/Transform.h"

//AudioListener* AudioListener::main = nullptr;

AudioListener::AudioListener()
{ }

AudioListener::~AudioListener()
{ }

AudioListener* AudioListener::clone() const
{
    return new AudioListener();
}

void AudioListener::update()
{
    vec3 p = tr->getPosition();
    vec3 d = tr->getDirection();

    sf::Listener::setPosition(p.x, p.z, p.y);
    sf::Listener::setDirection(d.x, d.z, d.y);
}
