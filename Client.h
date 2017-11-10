#pragma once

#include <MinGE.h>
#include "Board.h"
#include "definitions.h"

#include <thread>

class Player;

class Client: public Script
{
    public:
        void start() override;
        void update() override;
        void onDestroy() override;

        void connectToServer();
        void waitForSocket();

        void registerEvent(sf::Int8 _type, std::string& _players, std::string& _pieces);
        void registerEvent(sf::Int8 _type, sf::Int8 _idSrc, sf::Int8 _idDest);
        void registerEvent(sf::Int8 _type, sf::Int8 _id, vec2 _mp);
        void registerEvent(sf::Int8 _type, sf::Int8 _id);
        void registerEvent(sf::Int8 _type);

        void emitEvent(sf::Int8 _type, sf::Int8 _id, vec2 _mp);

        void receivePacket();
        void sendPacket();

    private:
        sf::SocketSelector selector;
        sf::TcpSocket socket;
        sf::Packet packetSending;

        std::thread selectorThread;
        bool packetReceived, loop;

        sf::Int8 id;

        Board* board;
        Entity* player;
        std::map<sf::Int8, Player*> players;

        unsigned short port = 53000;
        sf::IpAddress serverAddress = "192.168.0.12";
};
