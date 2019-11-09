#include <algorithm>

#include "Renderer/CommandBucket.h"
#include "Renderer/CommandKey.h"

#include "Assets/RenderTarget.h"

void CommandBucket::sort()
{
	std::sort(commands, commands + current_cmd, [](const CommandPair &a, const CommandPair &b) {
		return (a.key < b.key);
	});
}

void CommandBucket::submit()
{
	target->bind();

	for (unsigned i(0); i < current_cmd; ++i)
	{
		uint64_t key = commands[i].key;
		void *packet = commands[i].packet;

		CommandPacket::submit(key, packet);
	}
}

void CommandBucket::clear()
{
	for (int i(0); i < current_cmd; i++)
	{
		free(commands[i].packet);
	}
	current_view = 0;
	current_cmd = 0;
}
