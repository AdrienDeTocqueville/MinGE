#pragma once

#include "Core/UID.h"
#include "Graphics/GLDriver.h"

struct Texture: public UID<GLuint>
{
	Texture(): UID(0) {}

	static const Texture none;

private:
	Texture(uint32_t i): UID(i) {}
};
