#include "Network.h"

sf::IpAddress Network::publicIP;
sf::IpAddress Network::localIP;

void Network::init()
{
    publicIP = sf::IpAddress::getPublicAddress();
    localIP = sf::IpAddress::getLocalAddress();
}

sf::IpAddress Network::getPublicIP()
{
    return publicIP;
}

sf::IpAddress Network::getLocalIP()
{
    return localIP;
}
