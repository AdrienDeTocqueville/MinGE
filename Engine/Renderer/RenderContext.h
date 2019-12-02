#pragma once

#include "Utility/Memory/BlockAllocator.h"

struct RenderContext
{
	friend class GraphicEngine;

	template<typename Command>
	Command *create();

	void add(uint64_t key, void *cmd);
	void clear();

	inline size_t cmd_count() const
	{ return commands.count(); }

private:
	struct CommandPair
	{
		uint64_t key;
		void *packet;

		inline bool operator<(const CommandPair &other)
		{ return key < other.key; }
	};

	BlockAllocator<2048, CommandPair> commands;
	StackAllocator<4096> packets;
};
