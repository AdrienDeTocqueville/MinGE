#pragma once

#include <MinGE.h>
#include "Board.h"
#include "definitions.h"
#include "RemoteClient.h"

#include <thread>

class Server: public Script
{
    public:
        void start() override;
        void update() override;
        void onDestroy() override;

        void detectNewClient();

        void toString(std::string& _players, std::string& _pieces);

        void registerEvent(sf::Int8 _type, sf::Int8 _id, vec2 _mp);
        void registerEvent(sf::Int8 _type, sf::Int8 _id);

        void emitEvent(sf::TcpSocket* _socket, const sf::Int8& _type)
        {
            sf::Packet event;
            event << _type;

            send(_socket, event);
        }

        template <typename T>
        void emitEvent(sf::TcpSocket* _socket, const sf::Int8& _type, const T& _p1)
        {
            sf::Packet event;
            event << _type << _p1;

            send(_socket, event);
        }

        template <typename T, typename U>
        void emitEvent(sf::TcpSocket* _socket, const sf::Int8& _type, const U& _p1, const T& _p2)
        {
            sf::Packet event;
            event << _type << _p1 << _p2;

            send(_socket, event);
        }

        template<typename... Args>
        void emitEvent(const sf::Int8& _type, Args&&... args)
        {
            for (auto& it: clients)
                emitEvent(it.second.socket, _type, args...);
        }

        void receivePacket();

    private:
        void send(sf::TcpSocket* _socket, sf::Packet& event);

        struct RemoteClient
        {
            sf::TcpSocket* socket;
            Entity* player;
        };

        sf::TcpListener listener;
        bool localClient;

        std::thread selectorThread;

        std::map<sf::Int8, RemoteClient> clients;
        sf::Int8 nextId;
        sf::TcpSocket* nextClient;

        Board* board;
        Entity* player;

        unsigned short port = 53000;
};

