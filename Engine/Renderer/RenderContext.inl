#include "Renderer/RenderContext.h"
#include "Renderer/Commands.h"

template<typename Command>
Command *RenderContext::create()
{
	void *packet = packets.alloc((uint32_t)CommandPacket::getSize<Command>());
	CommandPacket::setSubmitCallback(packet, Command::submit);

	return CommandPacket::getCommand<Command>(packet);
}
