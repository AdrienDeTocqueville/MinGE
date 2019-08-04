#pragma once

#include <SFML/Network.hpp>

class Network
{
    friend class Engine;

    public:
        static sf::IpAddress getPublicIP();
        static sf::IpAddress getLocalIP();

    private:
        static void init();

        static sf::IpAddress publicIP, localIP;
};
