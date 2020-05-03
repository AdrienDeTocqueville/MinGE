#pragma once

#include <SFML/Network.hpp>

struct Network
{
	static sf::IpAddress public_ip() { return publicIP; }
	static sf::IpAddress local_ip() { return localIP; }

private:
	static void init();

	static sf::IpAddress publicIP, localIP;

	friend struct Engine;
};
