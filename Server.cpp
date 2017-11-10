#include "Server.h"
#include "RemotePlayer.h"

void Server::start()
{
    FileReader file("Data/network.txt");

    file["port"] >> port;

    std::cout << "Hosting server on: " << Network::getLocalIP() << ":" << port << std::endl;

    listener.listen(port);
    listener.setBlocking(false);

    localClient = false;

    nextId = 0;
    nextClient = new sf::TcpSocket();
    nextClient->setBlocking(false);

    player = Entity::findByTag("Player", true);

    insert<Board>();
    board = find<Board>();
}

void Server::update()
{
    detectNewClient();

    receivePacket();
}

void Server::onDestroy()
{
    if (clients.size())
    {
        emitEvent(SHUTDOWN);
        sf::sleep(sf::seconds(1.0f));
    }

    for (auto& it: clients)
        delete it.second.socket;

    delete nextClient;
}

void Server::detectNewClient()
{
    if (listener.accept(*nextClient) == sf::Socket::Done)
    {
        std::cout << (int)nextId << " joined the server." << std::endl;

        std::string playerString, piecesString;
        toString(playerString, piecesString);

        emitEvent(nextClient, JOIN, playerString, piecesString);
        emitEvent(CONNECT, nextId);


        Entity* np = Entity::clone(player)->insert<RemotePlayer>();

        RemotePlayer* rplayer = np->find<RemotePlayer>();
            rplayer->client = nullptr;
            rplayer->board = board;
            rplayer->id = nextId;

        clients[nextId] = {nextClient, np};

        nextId++;
        nextClient = new sf::TcpSocket();
        nextClient->setBlocking(false);
    }
}

void Server::toString(std::string& _players, std::string& _pieces)
{
    auto pieces = Entity::findAllByTag("Piece", false);

    _players.clear();
    _players.reserve(clients.size());

    _pieces.clear();
    _pieces.reserve(pieces.size()*2);

    for (auto& clientPair: clients)
        _players.push_back(clientPair.first);

    for (Entity* pieceEnt: pieces)
    {
        Piece* piece = pieceEnt->find<Piece>();

        _pieces.push_back(piece->id);
        _pieces.push_back(piece->selectedBy);

        if (piece->selectedBy == -1)
        {
            _pieces.push_back((char)Fleche(pieceEnt->find<Transform>()->getPosition()).team);
            _pieces.push_back((char)Fleche(pieceEnt->find<Transform>()->getPosition()).num);
        }
    }
}

void Server::registerEvent(sf::Int8 _type, sf::Int8 _id, vec2 _mp)
{
    if (_type == CLICK)
    {
        registerEvent(MOUSE, _id, _mp);

        RayHit hit = PhysicEngine::get()->raycast(vec3(_mp.x, _mp.y, 2.0f), vec3(0, 0, -1.0f));

        if (hit.distance != -1.0f)
        {
            RemotePlayer* rp = clients[_id].player->find<RemotePlayer>();

            if (rp->selected == nullptr)
            {
                Piece* piece = hit.collider->find<Piece>();

                if (piece != nullptr && piece->id != -1 && piece->selectedBy == -1)
                {
                    rp->select(piece->id);

                    emitEvent(SELECT, _id, piece->id);
                }
            }
            else
            {
                rp->unselect();

                emitEvent(UNSELECT, _id);
            }
        }
    }

    else if (_type == MOUSE)
    {
        clients[_id].player->find<RemotePlayer>()->updatePosition(_mp);

        for (auto& it: clients)
            if (it.first != _id)
                emitEvent(it.second.socket, MOUSE, _mp.x, _mp.y);
    }
}

void Server::registerEvent(sf::Int8 _type, sf::Int8 _id)
{
    if (_type == DISCONNECT)
    {
        std::cout << "PACKET DISCONNECT RECEIVED FROM" << (int)_id << std::endl;

        auto clientIt = clients.find(_id);

        if (clientIt != clients.end())
        {
            delete clientIt->second.socket;
            clientIt->second.player->destroy();
            clients.erase(clientIt);

            emitEvent(DISCONNECT, _id);

            std::cout << (int)_id << " left the server." << std::endl;

            for (Entity* pieceEnt: Entity::findAllByTag("Piece", false))
            {
                Piece* piece = pieceEnt->find<Piece>();

                if (piece->selectedBy == _id)
                    piece->selectedBy = -1;
            }
        }
    }
}

void Server::receivePacket()
{
    for (auto& it: clients)
    {
        sf::TcpSocket* clientSocket = it.second.socket;
        sf::Packet event;

        while (1)
        {
            auto status = clientSocket->receive(event);

            if (status == sf::Socket::NotReady)
                break;

            else if (status == sf::Socket::Partial)
                continue;

            else if (status == sf::Socket::Disconnected)
            {
                std::cout << "Client is unreachable" << std::endl;
                break;
            }

            else if (status == sf::Socket::Error)
            {
                std::cout << "Error status" << std::endl;
                break;
            }

            sf::Int8 type, idSrc;
            event >> type >> idSrc;

            if (type == CLICK || type == MOUSE)
            {
                vec2 mp;
                event >> mp.x >> mp.y;

                registerEvent(type, idSrc, mp);
            }

            else
                registerEvent(type, idSrc);
        }
    }
}

void Server::send(sf::TcpSocket* _socket, sf::Packet& event)
{
    while (_socket->send(event) == sf::Socket::Partial)
    { }
}
