#pragma once

#include <chrono>

struct Time
{
	static float delta_time;
	static float time;

	static uint32_t frame_duration(); // in ms

	struct Chrono
	{
		Chrono();
		long time() const;
		std::chrono::high_resolution_clock::time_point start;
	};

private:
	static void init();
	static void tick();

	friend struct Engine;
};
