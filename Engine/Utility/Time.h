#pragma once

class Time
{
	friend class Engine;

public:
	static float deltaTime;
	static float timeScale;
	static float time;

private:
	static void init();
	static void tick();
};
