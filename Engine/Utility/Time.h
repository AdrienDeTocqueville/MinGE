#pragma once

#include <chrono>

struct Time
{
	static float delta_time;
	static float time;

	static uint32_t frame_time; // in ms

	// time elapsed since the start of the frame
	static uint32_t frame_duration(); // in ms
	static void set_fps_max(uint32_t fps_max);

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
