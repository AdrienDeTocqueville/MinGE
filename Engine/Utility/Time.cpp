#include "Utility/Time.h"

#include <SFML/System/Clock.hpp>

static sf::Clock clock;

float Time::deltaTime = 0.0;
float Time::timeScale = 1.0;
float Time::time = 0.0;

void Time::init()
{
	deltaTime = 0.0f;
	timeScale = 1.0f;
	time = 0.0f;

	clock.restart();
}

void Time::tick()
{
	deltaTime = clock.restart().asSeconds() * timeScale;
	time += deltaTime;
}
