#pragma once

#include "Utility/helpers.h"

class UIView;
namespace sf {
	class Event;
}

class UISystem
{
	friend class Engine;
public:
	/// Methods (static)
	static UISystem* get() { return instance; }

	/// Methods (public)
	void addView(UIView* _view);
	void removeView(const UIView* _view);

	void on_event(const sf::Event &event);

private:
	/// Methods (private)
	UISystem();
	~UISystem();

	//void clear();

	static void create();
	static void destroy();

	void draw();

	/// Attributes (private)
	std::vector<UIView*> views;

	/// Attributes (static)
	static UISystem* instance;
};
