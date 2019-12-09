#pragma once

#include "Utility/glm.h"
#include <SFML/System/Clock.hpp>

namespace sf
{ class RenderWindow; }

class Engine
{
public:
	Engine(sf::RenderWindow* _window, unsigned _FPS = 60);
	~Engine();

	/// Methods (static)
	static Engine* get()	{ return instance; }

	/// Methods (public)
	void start();
	bool update();
	void clear();

	/// Setters
	void setPause(bool _pause);
	void togglePause();
	void setWindowSize(vec2 _newSize);

	/// Getters
	bool getPause() const;

private:
	/// Attributes
	sf::Clock clock;
	bool pause;

	/// Attributes (static)
	static Engine* instance;
};
