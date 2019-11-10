#include "Renderer/Commands.h"
#include "Renderer/CommandKey.h"

#include "Assets/Program.h"

void DrawElements::submit(uint64_t key, const void *_cmd)
{
	const DrawElements* cmd = reinterpret_cast<const DrawElements*>(_cmd);

	Program::setBuiltin("MATRIX_M", cmd->model);
	Program::setBuiltin("MATRIX_N", cmd->model);

	auto pass = CommandKey::decodeRenderPass(key);
	auto material = CommandKey::decodeMaterial(key);
	Material::get(material)->bind(pass);

	GL::BindVertexArray(cmd->vao);
	glCheck(glDrawElements(cmd->mode, cmd->count, GL_UNSIGNED_SHORT, (void*)(uint64_t)cmd->offset));
}

void SetupView::submit(uint64_t, const void *_cmd)
{
	const SetupView* cmd = reinterpret_cast<const SetupView*>(_cmd);
	CommandBucket::View *view = cmd->view;

	GL::Viewport(view->viewport);
	GL::Scissor (view->viewport);

	GL::ClearColor(view->clearColor);
	glClear(view->clearFlags);

	Program::setBuiltin("MATRIX_VP", view->vp);
}
