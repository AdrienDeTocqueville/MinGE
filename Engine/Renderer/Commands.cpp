#include "Renderer/Commands.h"
#include "Renderer/GLDriver.h"
#include "Renderer/CommandKey.h"

#include "Systems/GraphicEngine.h"
#include "Components/Light.h"
#include "Assets/Shader.h"
#include "Assets/Shader.inl"

#ifdef LOG_COMMANDS
# define LOG(x) std::cout << x << std::endl;
#else
# define LOG(x)
#endif

void DrawElements::submit(uint64_t key, const void *_cmd)
{
	LOG("DrawElements");

	auto pass = CommandKey::decodeRenderPass(key);
	auto material = CommandKey::decodeMaterial(key);
	const DrawElements* cmd = reinterpret_cast<const DrawElements*>(_cmd);

	Shader::setBuiltin("MATRIX_M", cmd->model);
	Shader::setBuiltin("MATRIX_N", cmd->model);

	if (pass == RenderPass::Forward)
		GraphicEngine::get()->getLight(0)->bind();

	Material::get(material)->bind(pass);

	const Submesh &submesh = cmd->submesh;
	GL::BindVertexArray(cmd->vao);
	glCheck(glDrawElements(submesh.mode, submesh.count, GL_UNSIGNED_SHORT, (void*)(uint64_t)submesh.offset));
}

void SetupView::submit(uint64_t, const void *_cmd)
{
	LOG("SetupView");

	const SetupView* cmd = reinterpret_cast<const SetupView*>(_cmd);
	View *view = cmd->view;

	GL::BindFramebuffer(view->fbo);

	Material::bound = NULL;
	Shader::setBuiltin("MATRIX_VP", view->vp);
	Shader::setBuiltin("VIEW_POS", view->view_pos);

	GL::Viewport(view->viewport);
	GL::Scissor (view->viewport);
	glEnable(GL_CULL_FACE);

	if (view->clear_flags & GL_COLOR_BUFFER_BIT)
	{
		GL::ClearColor(view->clear_color);
		glCullFace(GL_BACK);
	}
	else
		glCullFace(GL_FRONT);

	glClear(view->clear_flags);
}

void SetupSkybox::submit(uint64_t, const void *)
{
	LOG("SetupSkybox");

	GL::BindFramebuffer(0);
	glDisable(GL_CULL_FACE);
}
