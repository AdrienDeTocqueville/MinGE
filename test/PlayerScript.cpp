#include "PlayerScript.h"

PlayerScript::PlayerScript(float _speed, float _minSpeed, float _maxSpeed):
	speed(_speed), minSpeed(_minSpeed), maxSpeed(_maxSpeed)
{ }

void PlayerScript::update()
{
	float realSpeed = Input::getKeyDown(sf::Keyboard::LShift) ? maxSpeed: speed;
		  realSpeed = Input::getKeyDown(sf::Keyboard::LControl) ? minSpeed: realSpeed;

	vec3 direction = Camera::main->getDirection();
	vec3 right = normalize(cross(direction, vec3(0, 0, 1)));

	float force = 50.0f * rb->getMass() * Time::deltaTime * realSpeed;

	if (Input::getKeyDown(sf::Keyboard::Z))	 rb->applyForceToCOM( force * direction);
	if (Input::getKeyDown(sf::Keyboard::S))	 rb->applyForceToCOM(-force * direction);
	if (Input::getKeyDown(sf::Keyboard::D))	 rb->applyForceToCOM( force * right);
	if (Input::getKeyDown(sf::Keyboard::Q))	 rb->applyForceToCOM(-force * right);

	if (Input::getKeyDown(sf::Keyboard::Space))
	{
		rb->setLinearVelocity(vec3(0.0f));
		rb->setAngularVelocity(vec3(0.0f));
	}

	if (Input::getKeyDown(sf::Keyboard::O))
	{
		tr->position = vec3(0.0f, 0, 1);
		tr->rotation = quat(vec3(0.0f));
	}
}
