#pragma once

#include <cstdint>
#include <cstdlib>

struct CommandPacket
{
	typedef void (*submitCallback)(uint64_t, const void*);

	template <typename T>
	static void *create()
	{
		// TODO: use a linear allocator
		return malloc(sizeof(submitCallback) + sizeof(T));
	}

	static void submit(uint64_t key, void *packet)
	{
		auto callback = *reinterpret_cast<submitCallback*>(packet);
		callback(key, reinterpret_cast<uint8_t*>(packet) + sizeof(submitCallback));
	}

	static void setSubmitCallback(void *packet, submitCallback callback)
	{
		*reinterpret_cast<submitCallback*>(packet) = callback;
	}

	template <typename T>
	static T *getCommand(void *packet)
	{
		return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(packet) + sizeof(submitCallback));
	}
};
