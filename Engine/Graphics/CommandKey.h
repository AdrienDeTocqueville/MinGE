#pragma once

#include <limits>

struct CommandKey
{
	//TODO: add submesh to key for instancing
	static const uint8_t  MATERIAL_NUM_BITS	= 32;
	static const uint8_t  MATERIAL_SHIFT	= 64 - MATERIAL_NUM_BITS;
	static const uint64_t MATERIAL_MASK	= ((uint64_t(1) << MATERIAL_NUM_BITS) - 1) << MATERIAL_SHIFT;

	static const uint8_t  VARIANT_NUM_BITS	= 16;
	static const uint8_t  VARIANT_SHIFT	= MATERIAL_SHIFT - VARIANT_NUM_BITS;
	static const uint64_t VARIANT_MASK	= ((uint64_t(1) << VARIANT_NUM_BITS) - 1) << VARIANT_SHIFT;

	static const uint8_t  DEPTH_NUM_BITS	= 16;
	static const uint8_t  DEPTH_SHIFT	= VARIANT_SHIFT - DEPTH_NUM_BITS;
	static const uint64_t DEPTH_MASK	= ((uint64_t(1) << DEPTH_NUM_BITS) - 1) << DEPTH_SHIFT;

	static_assert(64 == MATERIAL_NUM_BITS + VARIANT_NUM_BITS + DEPTH_NUM_BITS,
		"Command key is not 64 bits");

	// Encode
	static uint64_t encode(uint32_t material, uint32_t variant, float depth)
	{
		int32_t depth_int;
		memcpy(&depth_int, &depth, sizeof(float));
		depth_int = (depth_int & 0x7FFFFF) >> (23 - DEPTH_NUM_BITS);

		return	((uint64_t(material)  << MATERIAL_SHIFT) & MATERIAL_MASK) |
			((uint64_t(variant)   << VARIANT_SHIFT)  & VARIANT_MASK)  |
			((uint64_t(depth_int) << DEPTH_SHIFT)    & DEPTH_MASK);
	}
};
