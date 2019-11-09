#include "Renderer/CommandBucket.h"
#include "Renderer/CommandKey.h"

#include "Assets/RenderTarget.h"

void CommandBucket::sort()
{}

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
