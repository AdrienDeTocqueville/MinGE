#include "PlayerScript.h"
#include "CameraScript.h"

PlayerScript::PlayerScript(float _speed, float _alt_speed):
	speed(_speed), alt_speed(_alt_speed)
{ }

void PlayerScript::update()
{
	speed = max(0.0f, speed - 4.0f * Input::getMouseWheelDelta());

	float realSpeed = Input::getKeyDown(sf::Keyboard::LShift) ? alt_speed: speed;
	float force = 50.0f * rb->getMass() * Time::deltaTime * realSpeed;

	vec3 direction = CameraScript::getMovement(cam->getDirection());
	rb->applyForceToCOM(force * direction);

	if (Input::getKeyDown(sf::Keyboard::P))
	{
		rb->setLinearVelocity(vec3(0.0f));
		rb->setAngularVelocity(vec3(0.0f));
	}

	if (Input::getKeyDown(sf::Keyboard::O))
	{
		tr->position = vec3(0.0f);
		tr->rotation = quat(vec3(0.0f));
	}
}
