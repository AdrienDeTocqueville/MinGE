#pragma once

#include "Assets/Material.h"

struct CommandKey
{
	static const uint8_t VIEW_SHIFT = 0, DEPTH_SHIFT = 0;
	static const uint64_t VIEW_MASK = 0, DEPTH_MASK = 0;

	// Encode
	static uint64_t encode(unsigned view, RenderPass pass, unsigned priority = 0)
	{
		return	((view << VIEW_SHIFT) & VIEW_MASK);
	}
	static uint64_t encode(unsigned view, RenderPass pass, unsigned material, float depth)
	{
		return	((view			<< VIEW_SHIFT)	& VIEW_MASK) |
			((uint64_t(depth)	<< DEPTH_SHIFT)	& DEPTH_MASK);
	}

	// Decode
	static RenderPass decodeRenderPass(uint64_t key)
	{
		return RenderPass::Forward;
	}
	static MaterialRef decodeMaterial(uint64_t key)
	{
		return nullptr;
	}
};
