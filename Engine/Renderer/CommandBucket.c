#include "Renderer/CommandBucket.h"
#include "Renderer/CommandKey.h"

/*
void CommandBucket::add(MeshRef mesh, unsigned submesh, MaterialRef material, float depth, const mat4& toWorld)
{
	CommandKey key;
	key.depth = depth;

	int index = allocCmd();
	keys[index] = key.encode();
	commands
}
*/

void CommandBucket::sort()
{}

void CommandBucket::submit()
{
	// Apply view state
	GL::Viewport(viewport);
	GL::Scissor (viewport);

	GL::BindFramebuffer(fbo);

	GL::ClearColor(clearColor);
	glClear(clearFlags);

	// TODO: bind vp to shader

	// Execute commands
}

void CommandBucket::clear()
{}
