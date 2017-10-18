#include "Components/AudioListener.h"
#include "Components/Transform.h"

AudioListener* AudioListener::main = nullptr;

AudioListener::AudioListener()
{ }

AudioListener::~AudioListener()
{ }

Component* AudioListener::clone()
{
    return new AudioListener();
}

void AudioListener::registerComponent()
{
    if (main != nullptr)
        delete main;

    main = this;
}

void AudioListener::deregisterComponent()
{
    main = nullptr;
}

void AudioListener::update()
{
    vec3 p = tr->getPosition();
    vec3 d = tr->getDirection();

    sf::Listener::setPosition(p.x, p.z, p.y);
    sf::Listener::setDirection(d.x, d.z, d.y);
}
