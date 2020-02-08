#include "Systems/UISystem.h"
#include "Components/UIView.h"

#include "Renderer/GLDriver.h"
#include "Profiler/profiler.h"
#include "Utility/IO/Input.h"

#include "UI/FileSystemBasic.h"
#include "UI/GPUDriverGL.h"
#include "UI/keycodes.h"

#include "Assets/Material.h"
#include "Assets/Mesh.h"
#include "Assets/Shader.h"
#include "Assets/Shader.inl"

#include <Ultralight/Ultralight.h>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

using ultralight::RefPtr;


UISystem* UISystem::instance = nullptr;

ultralight::FileSystemBasic *fs = nullptr;
ultralight::GPUDriverGL *driver = nullptr;
RefPtr<ultralight::Renderer> renderer = nullptr;

MeshRef fullscreen_quad;
MaterialRef ui_material;

struct Listener : public ultralight::LoadListener
{
	virtual void OnDOMReady(ultralight::View* caller)
	{
		for (auto view: UISystem::get()->views)
		{
			if (view->view == caller)
				view->dom_ready = true;
		}
	}
} listener;


/// Methods (private)
UISystem::UISystem()
{
	fullscreen_quad = Mesh::createQuad(MeshData::Points);
	ui_material = Material::create("ui");

	fs = new ultralight::FileSystemBasic("Assets/");
	driver = new ultralight::GPUDriverGL();

	ultralight::Platform::instance().set_file_system(fs);
	ultralight::Platform::instance().set_gpu_driver(driver);

	renderer = ultralight::Renderer::Create();
}

UISystem::~UISystem()
{
	renderer = nullptr;

	fullscreen_quad = nullptr;
	ui_material = nullptr;

	delete fs;
	delete driver;
}

void UISystem::clear()
{
	views.clear();
}

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

	auto ul_view = renderer->CreateView(size.x, size.y, false);
	ul_view->set_load_listener(&listener);
	ul_view->AddRef();

	_view->view = ul_view.ptr();
	views.push_back(_view);
}

void UISystem::removeView(const UIView* _view)
{
	for (unsigned i(0) ; i < views.size() ; i++)
	{
		if (views[i] == _view)
		{
			_view->view->Release();

			views[i] = views.back();
			views.pop_back();

			return;
		}
	}
}

void UISystem::on_event(const sf::Event &event)
{
	using ultralight::MouseEvent;
	using ultralight::ScrollEvent;
	using ultralight::KeyEvent;

	static MouseEvent::Button button_map[3] = {
		MouseEvent::Button::kButton_Left,
		MouseEvent::Button::kButton_Right,
		MouseEvent::Button::kButton_Middle,
	};

	MouseEvent m;
	ScrollEvent s;
	KeyEvent k;

	switch (event.type)
	{
	/// Mouse Event
	case sf::Event::MouseButtonPressed:
		m.type = MouseEvent::Type::kType_MouseDown;
		goto mouse_button_event;

	case sf::Event::MouseButtonReleased:
		m.type = MouseEvent::Type::kType_MouseUp;
		goto mouse_button_event;

	case sf::Event::MouseMoved:
		m.type = MouseEvent::Type::kType_MouseMoved;
		goto mouse_event;

	/// Keyboard Event
	case sf::Event::KeyPressed:
		k.type = KeyEvent::Type::kType_KeyDown;
		goto key_event;

	case sf::Event::KeyReleased:
		k.type = KeyEvent::Type::kType_KeyUp;
		goto key_event;

	case sf::Event::TextEntered:
		k.type = KeyEvent::Type::kType_Char;
		k.virtual_key_code = event.text.unicode;
		goto text_event;

	/// Resive Event
	case sf::Event::Resized:
		vec2 ws = vec2(event.size.width, event.size.height);
		for (int i(0); i < views.size(); i++)
		{
			ivec2 size = vec2(views[i]->viewport.z, views[i]->viewport.w) * ws;
			views[i]->view->Resize(size.x, size.y);
		}
		return;

	/// Scroll Event
	case sf::Event::MouseWheelScrolled:
		s.type = ScrollEvent::Type::kType_ScrollByPixel;
		s.delta_x = 0;
		s.delta_y = event.mouseWheelScroll.delta*100;

		for (auto ui_view : views)
			ui_view->view->FireScrollEvent(s);
		return;

	default: return;
	}

	mouse_button_event:
		m.button = button_map[event.mouseButton.button];
	mouse_event:
		vec2 ws(Input::getWindowSize());
		ivec2 mb = ivec2(event.mouseButton.x, event.mouseButton.y);
		for (int i(0); i < views.size(); i++)
		{
			vec4 vp = views[i]->viewport;
			ivec2 origin(vp.x * ws.x, ws.y - (vp.y + vp.w) * ws.y);

			m.x = mb.x - origin.x;
			m.y = mb.y - origin.y;

			views[i]->view->FireMouseEvent(m);
		}
		return;

	key_event:
		k.virtual_key_code = sf_ul(event.key.code);
	text_event:
		if (event.key.control)
			k.modifiers |= KeyEvent::Modifiers::kMod_CtrlKey;
		if (event.key.alt)
			k.modifiers |= KeyEvent::Modifiers::kMod_AltKey;
		if (event.key.shift)
			k.modifiers |= KeyEvent::Modifiers::kMod_ShiftKey;
		if (event.key.system)
			k.modifiers |= KeyEvent::Modifiers::kMod_MetaKey;
		char txt[] = {k.virtual_key_code, 0};
		k.native_key_code = k.virtual_key_code;
		k.unmodified_text = txt;
		k.text = k.unmodified_text;
		GetKeyIdentifierFromVirtualKeyCode(k.virtual_key_code, k.key_identifier);
		k.is_auto_repeat = false;
		k.is_system_key = false;
		k.is_keypad = false;

		for (auto ui_view : views)
			ui_view->view->FireKeyEvent(k);
		return;
}

void UISystem::draw()
{
	MICROPROFILE_SCOPEI("SYSTEM_UI", "draw");

	if (views.empty())
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
	const int uv_vp_loc = ui_material->get_location("uv_viewport");
	const Submesh submesh = fullscreen_quad->getSubmeshes()[0];

	for (unsigned i(0) ; i < views.size() ; i++)
	{
		auto rt = views[i]->view->render_target();
		vec2 uv_dim = vec2(rt.width / (float)rt.texture_width,
			rt.height / (float)rt.texture_height);

		ui_material->set(vp_loc, views[i]->viewport);
		ui_material->set(uv_vp_loc, rt.uv_coords);

		ui_material->bind(RenderPass::Forward);
		driver->BindUltralightTexture(rt.texture_id);

		glCheck(glDrawElements(submesh.mode, submesh.count,
			GL_UNSIGNED_SHORT, (void*)(uint64_t)submesh.offset));
	}
}
