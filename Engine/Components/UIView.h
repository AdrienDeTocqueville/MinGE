#pragma once

#include "Components/Component.h"

class UIView : public Component
{
public:
	UIView(vec4 _viewport);
	virtual ~UIView();

	/// Methods (public)
	virtual UIView* clone() const override;

	void update();

	/// Attributes
	vec4 viewport;

private:
	/// Methods (private)
	virtual void onRegister() override;
	virtual void onDeregister() override;
};
