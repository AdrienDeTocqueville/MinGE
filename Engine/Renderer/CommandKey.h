#pragma once

#include <limits>

#include "Renderer/CommandBucket.h"
#include "Assets/Material.h"

struct CommandKey
{
	static const uint8_t  VIEW_NUM_BITS	= 3; // CommandBucket::MAX_VIEWS = 8;
	static const uint8_t  VIEW_SHIFT	= 64 - VIEW_NUM_BITS;
	static const uint64_t VIEW_MASK		= ((uint64_t(1) << VIEW_NUM_BITS) - 1) << VIEW_SHIFT;

	static const uint8_t  PASS_NUM_BITS	= 2; // RenderPass::Count = 3;
	static const uint8_t  PASS_SHIFT	= VIEW_SHIFT - PASS_NUM_BITS;
	static const uint64_t PASS_MASK		= ((uint64_t(1) << PASS_NUM_BITS) - 1) << PASS_SHIFT;

	static const uint8_t  CMD_NUM_BITS	= 1; // bool
	static const uint8_t  CMD_SHIFT		= PASS_SHIFT - CMD_NUM_BITS;
	static const uint64_t CMD_MASK		= ((uint64_t(1) << CMD_NUM_BITS) - 1) << CMD_SHIFT;

	static const uint8_t  MATERIAL_NUM_BITS	= 26;
	static const uint8_t  MATERIAL_SHIFT	= CMD_SHIFT - MATERIAL_NUM_BITS;
	static const uint64_t MATERIAL_MASK	= ((uint64_t(1) << MATERIAL_NUM_BITS) - 1) << MATERIAL_SHIFT;

	static const uint8_t  DEPTH_NUM_BITS	= 32;
	static const uint8_t  DEPTH_SHIFT	= MATERIAL_SHIFT - DEPTH_NUM_BITS;
	static const uint64_t DEPTH_MASK	= ((uint64_t(1) << DEPTH_NUM_BITS) - 1) << DEPTH_SHIFT;

	static_assert(CommandBucket::MAX_VIEWS <= (1 << VIEW_NUM_BITS));
	static_assert(RenderPass::Count <= (1 << PASS_NUM_BITS));

	static_assert(64 == VIEW_NUM_BITS + PASS_NUM_BITS + CMD_NUM_BITS + MATERIAL_NUM_BITS + DEPTH_NUM_BITS);

	// Encode
	static uint64_t encode(uint32_t view, RenderPass pass, uint32_t priority = 0)
	{
		return	((uint64_t(view)     << VIEW_SHIFT)	& VIEW_MASK)	|
			((uint64_t(pass)     << PASS_SHIFT)	& PASS_MASK)	|
			((uint64_t(0)        << CMD_SHIFT)	& CMD_MASK)	|
			((uint64_t(priority)));
	}
	static uint64_t encode(uint32_t view, RenderPass pass, uint32_t material, float depth)
	{
		uint32_t depth_i = depth * std::numeric_limits<uint32_t>::max();

		return	((uint64_t(view)     << VIEW_SHIFT)	& VIEW_MASK)	|
			((uint64_t(pass)     << PASS_SHIFT)	& PASS_MASK)	|
			((uint64_t(1)        << CMD_SHIFT)	& CMD_MASK)	|
			((uint64_t(material) << MATERIAL_SHIFT)	& MATERIAL_MASK)|
			((uint64_t(depth_i)    << DEPTH_SHIFT)	& DEPTH_MASK);
	}

	// Decode
	inline static RenderPass decodeRenderPass(uint64_t key)
	{
		return RenderPass((key & PASS_MASK) >> PASS_SHIFT);
	}
	inline static uint32_t decodeMaterial(uint64_t key)
	{
		return (key & MATERIAL_MASK) >> MATERIAL_SHIFT;
	}
};
