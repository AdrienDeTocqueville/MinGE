#pragma once

#include <limits>

#include "Systems/GraphicEngine.h"
#include "Assets/Material.h"

struct CommandKey
{
	static const uint8_t  VIEW_NUM_BITS	= 3; // GraphicEngine::MAX_VIEWS = 8;
	static const uint8_t  VIEW_SHIFT	= 64 - VIEW_NUM_BITS;
	static const uint64_t VIEW_MASK		= ((uint64_t(1) << VIEW_NUM_BITS) - 1) << VIEW_SHIFT;

	static const uint8_t  PASS_NUM_BITS	= 2; // RenderPass::Count = 3;
	static const uint8_t  PASS_SHIFT	= VIEW_SHIFT - PASS_NUM_BITS;
	static const uint64_t PASS_MASK		= ((uint64_t(1) << PASS_NUM_BITS) - 1) << PASS_SHIFT;

	static const uint8_t  CMD_NUM_BITS	= 1; // bool (draw = 1, state change = 0)
	static const uint8_t  CMD_SHIFT		= PASS_SHIFT - CMD_NUM_BITS;
	static const uint64_t CMD_MASK		= ((uint64_t(1) << CMD_NUM_BITS) - 1) << CMD_SHIFT;

	static const uint8_t  MATERIAL_NUM_BITS	= 26;
	static const uint8_t  MATERIAL_SHIFT	= CMD_SHIFT - MATERIAL_NUM_BITS;
	static const uint64_t MATERIAL_MASK	= ((uint64_t(1) << MATERIAL_NUM_BITS) - 1) << MATERIAL_SHIFT;

	static const uint8_t  DEPTH_NUM_BITS	= 32;
	static const uint8_t  DEPTH_SHIFT	= MATERIAL_SHIFT - DEPTH_NUM_BITS;
	static const uint64_t DEPTH_MASK	= ((uint64_t(1) << DEPTH_NUM_BITS) - 1) << DEPTH_SHIFT;

	static_assert(GraphicEngine::MAX_VIEWS <= (1 << VIEW_NUM_BITS), "Too many MAX_VIEWS");
	static_assert(RenderPass::Count <= (1 << PASS_NUM_BITS), "Too many pass count");

	static_assert(64 == VIEW_NUM_BITS + PASS_NUM_BITS + CMD_NUM_BITS + MATERIAL_NUM_BITS + DEPTH_NUM_BITS,
		"Command key is not 64 bits");

	// Encode
	static uint64_t encode(uint32_t view, RenderPass::Type pass, uint32_t priority = 0)
	{
		return	((uint64_t(view)     << VIEW_SHIFT)	& VIEW_MASK)	|
			((uint64_t(pass)     << PASS_SHIFT)	& PASS_MASK)	|
			((uint64_t(0)        << CMD_SHIFT)	& CMD_MASK)	|
			((uint64_t(priority)));
	}
	static uint64_t encode(uint32_t view, RenderPass::Type pass, uint32_t material, float depth)
	{
		uint32_t depth_i = (uint32_t)(depth * (float)std::numeric_limits<uint32_t>::max());

		return	((uint64_t(view)     << VIEW_SHIFT)	& VIEW_MASK)	|
			((uint64_t(pass)     << PASS_SHIFT)	& PASS_MASK)	|
			((uint64_t(1)        << CMD_SHIFT)	& CMD_MASK)	|
			((uint64_t(material) << MATERIAL_SHIFT)	& MATERIAL_MASK)|
			((uint64_t(depth_i)    << DEPTH_SHIFT)	& DEPTH_MASK);
	}

	// Decode
	inline static RenderPass::Type decodeRenderPass(uint64_t key)
	{
		return RenderPass::Type((key & PASS_MASK) >> PASS_SHIFT);
	}
	inline static uint32_t decodeMaterial(uint64_t key)
	{
		return (key & MATERIAL_MASK) >> MATERIAL_SHIFT;
	}
};
