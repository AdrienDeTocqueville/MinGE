#pragma once

#include <cstdint>
#include <cstdlib>

#include "Assets/Mesh.h"

struct CommandPacket
{
	typedef void (*submitCallback)(uint64_t, const void*);

	template <typename T>
	inline static size_t getSize()
	{
		return sizeof(submitCallback) + sizeof(T);
	}

	static void submit(uint64_t key, void *packet)
	{
		auto callback = *reinterpret_cast<submitCallback*>(packet);
		callback(key, reinterpret_cast<uint8_t*>(packet) + sizeof(submitCallback));
	}

	inline static void setSubmitCallback(void *packet, submitCallback callback)
	{
		*reinterpret_cast<submitCallback*>(packet) = callback;
	}

	template <typename T>
	inline static T *getCommand(void *packet)
	{
		return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(packet) + sizeof(submitCallback));
	}

	inline static void *fromCommand(void *command)
	{
		return reinterpret_cast<uint8_t*>(command) - sizeof(submitCallback);
	}
};

struct DrawElements
{
	static void submit(uint64_t key, const void *_cmd);

	mat4 model;

	uint32_t vao;
	Submesh submesh;
};

struct SetupView
{
	static void submit(uint64_t key, const void *_cmd);

	struct View *view;
};

struct SetupSkybox
{
	static void submit(uint64_t key, const void *_cmd);
};

/*
struct SetBuiltin
{
	static void submit(const void *_cmd);

};
*/
