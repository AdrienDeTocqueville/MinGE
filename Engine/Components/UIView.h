#pragma once

#include "Components/Component.h"
#include "UI/JSWrapper.h"

namespace ultralight {
	class View;
}

class UIView : public Component
{
	friend class UISystem;
	friend struct Listener;

public:
	UIView(const std::string& _url, vec4 _viewport);
	virtual ~UIView();

	/// Methods (public)
	virtual UIView* clone() const override;

	void load(const std::string& _url);
	bool is_dom_ready() const { return dom_ready; }

	JSContext get_js_context() const;

private:
	/// Methods (private)
	virtual void onRegister() override;
	virtual void onDeregister() override;

	/// Attributes
	vec4 viewport;
	ultralight::View *view;
	std::string url;

	bool dom_ready;
};
