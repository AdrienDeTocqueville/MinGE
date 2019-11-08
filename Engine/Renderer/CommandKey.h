#pragma once

struct CommandKey
{
	static const uint8_t VIEW_SHIFT = 0, DEPTH_SHIFT = 0;
	static const uint64_t VIEW_MASK = 0, DEPTH_MASK = 0;


	CommandKey() {}

	// Decode
	CommandKey(uint64_t key)
	{
		view = (key & VIEW_MASK) >> VIEW_SHIFT;
		depth = (key & DEPTH_MASK) >> DEPTH_SHIFT;
	}

	uint64_t encode()
	{
		return	(view << VIEW_SHIFT & VIEW_MASK) |
			(depth << DEPTH_SHIFT & DEPTH_MASK);
	}

	int view, depth;
};
