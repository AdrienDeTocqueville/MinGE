#include "Renderer/Commands.h"
#include "Renderer/GLDriver.h"
#include "Renderer/CommandKey.h"

#include "Systems/GraphicEngine.h"
#include "Assets/Shader.h"
#include "Assets/Shader.inl"

void DrawElements::submit(uint64_t key, const void *_cmd)
{
	const DrawElements* cmd = reinterpret_cast<const DrawElements*>(_cmd);

	Shader::setBuiltin("MATRIX_M", cmd->model);
	Shader::setBuiltin("MATRIX_N", cmd->model);

	auto pass = CommandKey::decodeRenderPass(key);
	auto material = CommandKey::decodeMaterial(key);
	Material::get(material)->bind(pass);

	const Submesh &submesh = cmd->submesh;
	GL::BindVertexArray(cmd->vao);
	glCheck(glDrawElements(submesh.mode, submesh.count, GL_UNSIGNED_SHORT, (void*)(uint64_t)submesh.offset));
}

void SetupView::submit(uint64_t, const void *_cmd)
{
	const SetupView* cmd = reinterpret_cast<const SetupView*>(_cmd);
	View *view = cmd->view;
	GL::BindFramebuffer(view->fbo);

	Shader::setBuiltin("MATRIX_VP", view->vp);
	GL::Viewport(view->viewport);
	GL::Scissor (view->viewport);

	GL::ClearColor(view->clearColor);
	glClear(view->clearFlags);

	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
}

void SetupSkybox::submit(uint64_t, const void *)
{
	glDisable(GL_CULL_FACE);
}
