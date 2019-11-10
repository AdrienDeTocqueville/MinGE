#include <algorithm>

#include "Renderer/CommandBucket.h"
#include "Renderer/CommandKey.h"

#include "Assets/RenderTarget.h"

void CommandBucket::sort()
{
	// TODO: parallel sort
	CommandPair *first = (CommandPair*)commands.getStart();
	CommandPair *last = (CommandPair*)(commands.getStart() + commands.getSize());
	std::sort(first, last, [](const CommandPair &a, const CommandPair &b) {
		return (a.key < b.key);
	});
}

void CommandBucket::submit()
{
	target->bind();

	uint8_t *first = commands.getStart();
	size_t count = commands.getSize();
	for (size_t i(0); i < count; i += sizeof(CommandPair))
	{
		CommandPair *pair = (CommandPair*)(first + i);
		uint64_t key = pair->key;
		void *packet = pair->packet;

		CommandPacket::submit(key, packet);
	}
}

void CommandBucket::clear()
{
	current_view = 0;

	commands.clear();
	packets.clear();
}
