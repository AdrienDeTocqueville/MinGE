#pragma once

#include "Components/Component.h"

namespace ultralight {
	class View;
}

class UIView : public Component
{
	friend class UISystem;

public:
	UIView(const std::string& _url, vec4 _viewport);
	virtual ~UIView();

	/// Methods (public)
	virtual UIView* clone() const override;

private:
	/// Methods (private)
	virtual void onRegister() override;
	virtual void onDeregister() override;

	/// Attributes
	vec4 viewport;
	ultralight::View *view;
	std::string url;
};
