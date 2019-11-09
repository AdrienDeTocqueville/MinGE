#pragma once

#include "Renderer/Commands.h"
#include "Renderer/CommandKey.h"

#include "Assets/Material.h"
#include "Assets/Mesh.h"

struct CommandBucket
{
	struct View
	{
		uint8_t passes;

		mat4 vp;
		vec4 viewport;
		vec4 clearColor;
		unsigned clearFlags;
	};

	struct CommandPair
	{
		uint64_t key;
		void *packet;
	};

	template<typename Cmd>
	Cmd *add(uint64_t key)
	{
		void *packet = CommandPacket::create<Cmd>();

		uint32_t index = current_cmd++;
		commands[index].key = key;
		commands[index].packet = packet;

		CommandPacket::setSubmitCallback(packet, Cmd::submit);
		return CommandPacket::getCommand<Cmd>(packet);
	}

	uint32_t add_view() { return current_view++; }
	View *get_view(uint32_t index) { return views + index; }

	void sort();
	void submit();
	void clear();

//private:
	class RenderTarget *target;


	static const size_t MAX_VIEWS = 8;
	uint32_t current_view = 0;
	View views[MAX_VIEWS];

	static const size_t MAX_COMMANDS = (64 << 10); // from bgfx, quite big but i guess its necessary
	uint32_t current_cmd = 0;
        CommandPair commands[MAX_COMMANDS];

	// TODO: use this for multithreaded counters
	// (and use an allocator instead of malloc/free for packets)
        //std::atomic<uint32_t> current_cmd;
};

struct DrawElements
{
	static void submit(uint64_t key, const void *_cmd);

	mat4 model;

	unsigned vao;
	GLdouble mode;
	unsigned count;
	void *offset;
};

struct SetupView
{
	static void submit(uint64_t key, const void *_cmd);

	CommandBucket::View *view;
};

/*
struct SetBuiltin
{
	static void submit(const void *_cmd);

};
*/
