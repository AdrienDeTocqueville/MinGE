#pragma once

class CommandPacket
{
	typedef void (*submitCallback)(const void*);

	template <typename T>
	static void *create()
	{
		// TODO: use a linear allocator
		return malloc(sizeof(submitCallback) + sizeof(T));
	}

	static void setSubmitCallback(void *packet, submitCallback callback)
	{
		*reinterpret_cast<submitCallback*>(packet) = callback;
	}

	static submitCallback getSubmitCallback(void *packet)
	{
		return *reinterpret_cast<submitCallback*>(packet);
	}

	template <typename T>
	static T *getCommand(void *packet)
	{
		return reinterpret_cast<T>(packet + sizeof(submitCallback));
	}
};

struct DrawCmd
{
	static void submit(const void *cmd);

	class Material *mat;
	unsigned vao;

	const GLdouble mode;
	const unsigned count;
	const void *offset;
};

