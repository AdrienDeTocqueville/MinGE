#include "Systems/UISystem.h"
#include "Components/UIView.h"

#include "Renderer/GLDriver.h"
#include "Profiler/profiler.h"
#include "Utility/IO/Input.h"
#include "UI/GPUDriverGL.h"

#include "Assets/Material.h"
#include "Assets/Mesh.h"
#include "Assets/Shader.h"
#include "Assets/Shader.inl"

#include <Ultralight/Ultralight.h>
#include <SFML/Graphics/RenderWindow.hpp>

using ultralight::RefPtr;

UISystem* UISystem::instance = nullptr;

ultralight::GPUDriverGL *driver = nullptr;
RefPtr<ultralight::Renderer> renderer = nullptr;

std::vector<RefPtr<ultralight::View>> ul_views;

MeshRef fullscreen_quad;
MaterialRef ui_material;

const char* htmlString() {
	return R"(
    <html>
      <head>
        <style type="text/css">
          body {
            margin: 0;
            padding: 0;
            overflow: hidden;
            color: black;
            font-family: Arial;
            background: linear-gradient(-45deg, #acb4ff, #f5d4e2);
          }
          div {
            width: 80%;
            height: 80%;
            text-align: center;
            margin: auto;
            border-radius: 25px;
            background: linear-gradient(-45deg, #e5eaf9, #f9eaf6);
            box-shadow: 0 7px 18px -6px #8f8ae1;
          }
          h1 {
            padding: 1em;
          }
          p {
            background: white;
            padding: 2em;
            margin: 40px;
            border-radius: 25px;
          }
        </style>
      </head>
      <body>
        <div>
          <h1>Hello World!</h1>
          <p>gpu!</p>
        </div>
      </body>
    </html>
    )";
}

/*
struct Listener : public ultralight::LoadListener
{
	bool finished_loading = false;

	virtual void OnBeginLoading(ultralight::View* caller)
	{ finished_loading = false; }
	virtual void OnFinishLoading(ultralight::View *caller)
	{ finished_loading = true; }
};
*/

/// Methods (private)
UISystem::UISystem()
{
	fullscreen_quad = Mesh::createQuad(MeshData::Points);
	ui_material = Material::create("ui");
	driver = new ultralight::GPUDriverGL();

	ultralight::Config config;
	config.device_scale_hint = 2.0;
	config.font_family_standard = "Arial";

	ultralight::Platform::instance().set_config(config);
	ultralight::Platform::instance().set_gpu_driver(driver);

	renderer = ultralight::Renderer::Create();

	//view->set_load_listener(&listener);
}

UISystem::~UISystem()
{
	ul_views.clear();
	renderer = nullptr;

	fullscreen_quad = nullptr;
	ui_material = nullptr;

	delete driver;
}

//void UISystem::clear()
//{
//	views.clear();
//}

/// Methods (static)
void UISystem::create()
{
	if (instance != nullptr)
		return;

	instance = new UISystem();
}

void UISystem::destroy()
{
	delete instance;
	instance = nullptr;
}

/// Methods (public)
void UISystem::addView(UIView* _view)
{
	vec2 vp(_view->viewport.z, _view->viewport.w);
	ivec2 size = vp * vec2(Input::getWindowSize());

	views.push_back(_view);
	ul_views.push_back(renderer->CreateView(size.x, size.y, false));

	ul_views.back()->LoadHTML(htmlString());
}

void UISystem::removeView(const UIView* _view)
{
	for (unsigned i(0) ; i < views.size() ; i++)
	{
		if (views[i] == _view)
		{
			views[i] = views.back();
			views.pop_back();
			ul_views[i] = ul_views.back();
			ul_views.pop_back();

			return;
		}
	}
}

void UISystem::draw()
{
	MICROPROFILE_SCOPEI("SYSTEM_UI", "draw");

	if (ul_views.empty())
		return;

	renderer->Update();

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	{
		renderer->Render();
		if (driver->HasCommandsPending())
			driver->DrawCommandList();
	}
	glPopAttrib();

	GL::BindFramebuffer(0);
	GL::BindVertexArray(fullscreen_quad->getVAO());

	const int vp_loc = ui_material->get_location("viewport");
	const int dim_loc = ui_material->get_location("uv_dim");
	const Submesh submesh = fullscreen_quad->getSubmeshes()[0];

	for (unsigned i(0) ; i < views.size() ; i++)
	{
		auto rt = ul_views[i]->render_target();
		vec2 uv_dim = vec2(rt.width / (float)rt.texture_width,
			rt.height / (float)rt.texture_height);

		ui_material->set(vp_loc, views[i]->viewport);
		ui_material->set(dim_loc, uv_dim);

		ui_material->bind(RenderPass::Forward);
		driver->BindUltralightTexture(rt.texture_id);

		glCheck(glDrawElements(submesh.mode, submesh.count,
			GL_UNSIGNED_SHORT, (void*)(uint64_t)submesh.offset));
	}
}
