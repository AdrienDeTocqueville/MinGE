#pragma once

#include <chrono>

class Time
{
	friend class Engine;

public:
	static float delta_time;
	static float time;

	struct Chrono
	{
		Chrono();
		long time() const;
		std::chrono::steady_clock::time_point start;
	};

private:
	static void init();
	static void tick();
};
