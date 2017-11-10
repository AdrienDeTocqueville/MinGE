#include "Utility/Time.h"

float Time::deltaTime = 0.0;
float Time::timeScale = 1.0;

void Time::init()
{
    deltaTime = 0.0f;
    timeScale = 1.0f;
}
