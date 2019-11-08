#pragma once

#include "Renderer/Commands.h"
#include "Renderer/CommandKey.h"

#include "Assets/RenderTarget.h"
#include "Assets/Material.h"
#include "Assets/Mesh.h"

struct CommandBucket
{
	template<typename Cmd>
	Cmd *add(uint64_t key)
	{
		void *packet = CommandPacket::create<Cmd>();

		uint32_t index = current_index++;
		commands[index].key = key;
		commands[index].packet = packet;

		CommandPacket::setSubmitCallback(packet, Cmd::submit);
		return CommandPacket::getCommand<Cmd>(packet);
	}

	template<typename Cmd>
	void add(Mesh *mesh, int submesh, Material *material, float depth, const mat4 &toWorld)
	{
		CommandKey key;
		/*
		key.view_id = 0;
		key.pass = 0;
		key.cmd = 0;
		key.material = 0;
		key.depth = 0;
		*/

		Cmd *cmd = add<Cmd>(key.encode());
	}

	void sort();
	void submit();
	void clear();

	// View state
	static const size_t MAX_VIEW = 8;
	struct View
	{
		mat4 vp;
		vec4 viewport;
		vec4 clearColor;
		unsigned clearFlags;
		RenderPass pass;
	};

	RenderTargetRef target;
	View views[MAX_VIEW];
	int viewCount;

private:
	static const size_t MAX_COMMANDS = 256;
	struct CommandPair
	{
		uint64_t key;
		void *packet;
	};

        //std::atomic<uint32_t> current_index;
	uint32_t current_index = 0;
        CommandPair commands[MAX_COMMANDS];
};
