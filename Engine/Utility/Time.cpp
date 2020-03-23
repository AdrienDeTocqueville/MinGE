#include "Utility/Time.h"

#include <SFML/System/Clock.hpp>
#include <iostream>

static sf::Clock _clock;

float Time::deltaTime = 0.0;
float Time::timeScale = 1.0;
float Time::time = 0.0;

void Time::init()
{
	deltaTime = 0.0f;
	timeScale = 1.0f;
	time = 0.0f;

	_clock.restart();
}

void Time::tick()
{
	deltaTime = _clock.restart().asSeconds() * timeScale;
	time += deltaTime;
}

Time::Chrono::Chrono()
{
	start = std::chrono::high_resolution_clock::now();
}
long Time::Chrono::time() const
{
	const auto end = std::chrono::high_resolution_clock::now();
        const auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	return diff.count();
}
