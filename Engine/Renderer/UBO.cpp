#include "Renderer/UBO.h"

#ifdef DEBUG
#include <iostream>
#endif

static inline uint32_t align(uint32_t val, uint32_t alignment)
{
	return ((val + alignment - 1) / alignment ) * alignment;
}

static GLint ubo_alignment = 0;
static uint32_t buf_size = 1024*1024*16;
void UBO::setupPool()
{
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &ubo_alignment);

	buf_size = align(buf_size, ubo_alignment);
}

struct PoolBuffer
{
	GLuint res;
	uint32_t used, allocated;
	uint8_t *data;
	unsigned refs;

	PoolBuffer(GLuint _res, uint32_t _used, uint32_t _allocated, uint8_t *_data, unsigned _refs):
		res(_res), used(_used), allocated(_allocated), data(_data), refs(_refs)
	{}
};
static std::vector<PoolBuffer> pool;

UBO UBO::create(uint32_t size)
{
	UBO ubo;

#ifdef DEBUG
	if (size >= buf_size)
		std::cout << "UBO is too big" << std::endl;
#endif

	for (PoolBuffer& buf : pool)
	{
		if (buf.used + size < buf.allocated)
		{
			ubo.res = buf.res;
			ubo.offset = align(buf.used, ubo_alignment);
			ubo.size = size;
			ubo.data = buf.data + ubo.offset;

			buf.used = ubo.offset + size;
			buf.refs++;
			return ubo;
		}
	}

	glGenBuffers(1, &ubo.res);
	GL::BindUniformBuffer(ubo.res);

	glBufferStorage(GL_UNIFORM_BUFFER, buf_size, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
	ubo.data = (uint8_t*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, buf_size, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);


	pool.emplace_back(ubo.res, size, buf_size, ubo.data, 0);

	ubo.offset = 0;
	ubo.size = size;
	return ubo;
}
