#include "Components/UIView.h"
#include "Systems/UISystem.h"

#include <Ultralight/Ultralight.h>

UIView::UIView(const std::string& _url, vec4 _viewport):
	viewport(_viewport), view(nullptr), url(_url)
{ }

UIView::~UIView()
{ }

UIView* UIView::clone() const
{
	return new UIView(url, viewport);
}

void UIView::load(const std::string& _url)
{
	url = _url;
	dom_ready = false;

	view->LoadURL(url.c_str());
}

void UIView::onRegister()
{
	UISystem::get()->addView(this);
	view->LoadURL(url.c_str());
}

void UIView::onDeregister()
{
	UISystem::get()->removeView(this);
}

