#include "Renderer/RenderContext.h"
#include "Renderer/Commands.h"

void RenderContext::add(uint64_t key, void *cmd)
{
	void *packet = CommandPacket::fromCommand(cmd);

	CommandPair *command = commands.alloc();
	command->key = key;
	command->packet = packet;
}

void RenderContext::clear()
{
	commands.clear();
	packets.clear();
}
