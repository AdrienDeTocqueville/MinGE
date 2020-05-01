#include "Utility/Time.h"

#include <SFML/System/Clock.hpp>
#include <iostream>

static sf::Clock _clock;

float Time::delta_time = 0.0;
float Time::time = 0.0;

void Time::init()
{
	delta_time = 0.0f;
	time = 0.0f;

	_clock.restart();
}

void Time::tick()
{
	delta_time = _clock.restart().asSeconds();
	time += delta_time;
}

Time::Chrono::Chrono()
{
	start = std::chrono::high_resolution_clock::now();
}
long Time::Chrono::time() const
{
	const auto end = std::chrono::high_resolution_clock::now();
	const auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	return (long)diff.count();
}
