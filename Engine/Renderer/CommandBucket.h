#pragma once

#include "Renderer/Commands.h"

#include "Assets/Material.h"
#include "Assets/Mesh.h"

class RenderTarget;
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

	CommandBucket(RenderTarget *_target, size_t _size = (1<<20)):
		target(_target), commands(_size), packets(_size)
	{ }

	// Thread safe
	template<typename Cmd>
	Cmd *add(uint64_t key)
	{
		void *packet = CommandPacket::create<Cmd>(packets);
		CommandPacket::setSubmitCallback(packet, Cmd::submit);

		CommandPair *command = (CommandPair*)commands.alloc(sizeof(CommandPair));
		command->key = key;
		command->packet = packet;

		return CommandPacket::getCommand<Cmd>(packet);
	}

	// Not thread safe
	uint32_t add_view() { return current_view++; }
	View *get_view(uint32_t index) { return views + index; }

	void sort();
	void submit();
	void clear();

//private:
	RenderTarget *target;

	static const size_t MAX_VIEWS = 8;
	uint32_t current_view = 0;
	View views[MAX_VIEWS];

        LinearAllocator commands, packets;
};

struct DrawElements
{
	static void submit(uint64_t key, const void *_cmd);

	mat4 model;

	uint32_t vao;
	uint32_t mode;
	uint32_t count;
	uint32_t offset;
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
