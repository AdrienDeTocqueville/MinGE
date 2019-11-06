#pragma once

struct CommandKey
{
	enum Type { SortMaterial, SortDepth };

	static const uint8_t VIEW_SHIFT_MAT = 0, DEPTH_SHIFT_MAT = 0;
	static const uint64_t VIEW_MASK_MAT = 0, DEPTH_MASK_MAT = 0;

	static const uint8_t VIEW_SHIFT_DEPTH = 0, DEPTH_SHIFT_DEPTH = 0;
	static const uint64_t VIEW_MASK_DEPTH = 0, DEPTH_MASK_DEPTH = 0;


	CommandKey() {}

	// Decode
	CommandKey(uint64_t key, Type type)
	{
		switch (type)
		{
		case SortMaterial:
			view = (key & VIEW_MASK_MAT) >> VIEW_SHIFT_MAT;
			depth = (key & DEPTH_MASK_MAT) >> DEPTH_SHIFT_MAT;
			break;


		case SortDepth:
			view = (key & VIEW_MASK_DEPTH) >> VIEW_SHIFT_DEPTH;
			depth = (key & DEPTH_MASK_DEPTH) >> DEPTH_SHIFT_DEPTH;
			break;
		}
	}

	uint64_t encode(Type type)
	{
		uint64_t key;
		switch (type)
		{
		case SortMaterial:
			key = (view << VIEW_SHIFT_MAT & VIEW_MASK_MAT) |
				(depth << DEPTH_SHIFT_MAT & DEPTH_MASK_MAT);
			return key;

		case SortDepth:
			key = (view << VIEW_SHIFT_DEPTH & VIEW_MASK_DEPTH) |
				(depth << DEPTH_SHIFT_DEPTH & DEPTH_MASK_DEPTH);
			return key;
		}
	}

	int view, depth;
};
