#include "Components/UIView.h"
#include "Systems/UISystem.h"

UIView::UIView(vec4 _viewport):
	viewport(_viewport)
{ }

UIView::~UIView()
{ }

UIView* UIView::clone() const
{
	return new UIView(viewport);
}

void UIView::update()
{
}
void UIView::onRegister()
{
	UISystem::get()->addView(this);
}

void UIView::onDeregister()
{
	UISystem::get()->removeView(this);
}

