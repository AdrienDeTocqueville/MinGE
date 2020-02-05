#pragma once

#include "Utility/helpers.h"

class UIView;
class UISystem
{
	friend class Engine;
public:
	/// Methods (static)
	static UISystem* get() { return instance; }


	/// Methods (public)
	void addView(UIView* _view);
	void removeView(const UIView* _view);

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
