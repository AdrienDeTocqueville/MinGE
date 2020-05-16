#include "Utility/Time.h"

#include <chrono>

using namespace std::chrono;

static time_point<high_resolution_clock> prev;

float Time::delta_time = 0.0f;
float Time::time = 0.0f;

void Time::init()
{
	delta_time = 0.0f;
	time = 0.0f;

	prev = high_resolution_clock::now();
}

void Time::tick()
{
	auto now = high_resolution_clock::now();
	delta_time = duration<float>(now - prev).count();
	time += delta_time;
	prev = now;
}

uint32_t Time::frame_duration()
{
	auto now = high_resolution_clock::now();
	auto test = duration_cast<milliseconds>(now - prev);
	return test.count();
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
