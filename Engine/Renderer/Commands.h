#pragma once

struct DrawCmd
{
	static void submit(DrawCmd *cmd);

	class Material *mat;
	unsigned vao;

	const GLdouble mode;
	const unsigned count;
	const void *offset;
};

