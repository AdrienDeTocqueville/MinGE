#include "Client.h"
#include "LocalPlayer.h"
#include "RemotePlayer.h"

void Client::start()
{
    FileReader file("Data/network.txt");

    file["adresse"] >> serverAddress;
    file["port"] >> port;

    std::cout << "Trying to join: " << serverAddress.toString() << ":" << port << std::endl;

    socket.setBlocking(false);

    packetReceived = false;
    loop = false;

    id = -2;

    player = Entity::findByTag("Player", true);
}

void Client::update()
{
    if (id == -2)
        connectToServer();

    if (packetReceived)
        receivePacket();
}

void Client::onDestroy()
{
    if (loop)
    {
        loop = false;
        selectorThread.join();

        packetSending << DISCONNECT << id;
        sendPacket();

        sf::sleep(sf::seconds(1.0f));
    }
}

void Client::connectToServer()
{
    if (socket.connect(serverAddress, port) == sf::Socket::Done)
    {
        std::cout << "Successfully connected at: " << socket.getRemoteAddress().toString() << std::endl;

        id = -1;
        loop = true;

        socket.setBlocking(true);
        selector.add(socket);
        selectorThread = std::thread(&Client::waitForSocket, this);

        insert<Board>();
        board = find<Board>();
    }
}

void Client::waitForSocket()
{
    while (loop)
    {
        if (selector.wait(sf::seconds(1.0f)))
        {
            packetReceived = true;
        }
    }
}

void Client::registerEvent(sf::Int8 _type, std::string& _players, std::string& _pieces)
{
    if (_type == JOIN)
    {
        std::cout << _players.size() << " players already connected." << std::endl;

        for (const sf::Int8& c: _players)
        {
            players[c] = Entity::clone(player)->insert<RemotePlayer>()->find<RemotePlayer>();
            players[c]->client = this;
            players[c]->board = board;
            players[c]->id = c;
        }

        for (unsigned i(0) ; i < _pieces.length() ; i += 2)
        {
            std::cout << i << ": ";
            std::cout << (int)_pieces[i] << ", ";
            std::cout << (int)_pieces[i+1] << std::endl;

            Piece* p = board->getPiece(_pieces[i]);

            if (_pieces[i+1] == -1)
            {
                std::cout << (int)_pieces[i+2] << ", ";
                std::cout << (int)_pieces[i+3] << std::endl;
                board->moveChecker(p->getEntity(), Fleche(_pieces[i+2], _pieces[i+3]));
                i += 2;
            }
            else
            {
                players[_pieces[i+1]]->select(_pieces[i]);
            }
        }
    }
}

void Client::registerEvent(sf::Int8 _type, sf::Int8 _idSrc, sf::Int8 _idDest)
{
    if (_type == SELECT)
        players[_idSrc]->select(_idDest);
}

void Client::registerEvent(sf::Int8 _type, sf::Int8 _id, vec2 _mp)
{
    if (_type == MOUSE)
        players[_id]->updatePosition(_mp);
}

void Client::registerEvent(sf::Int8 _type, sf::Int8 _id)
{
    if (_type == CONNECT)
    {
        std::cout << "Connect event" << std::endl;

        if (id == -1)
        {
            std::cout << "You are assigned id: " << (int)_id << std::endl;

            id = _id;
            players[_id] = Entity::clone(player)->insert<LocalPlayer>()->find<LocalPlayer>();
            players[_id]->client = this;
            players[_id]->board = board;
            players[_id]->id = _id;
        }
        else
        {
            std::cout << (int)_id << " joined the server." << std::endl;

            players[_id] = Entity::clone(player)->insert<RemotePlayer>()->find<RemotePlayer>();
            players[_id]->client = this;
            players[_id]->board = board;
            players[_id]->id = _id;
        }
    }

    else if (_type == DISCONNECT)
    {
        auto playerIt = players.find(_id);

        if (playerIt != players.end())
        {
            playerIt->second->getEntity()->destroy();
            players.erase(playerIt);

            std::cout << (int)_id << " left the server." << std::endl;

            for (Entity* pieceEnt: Entity::findAllByTag("Piece", false))
            {
                Piece* piece = pieceEnt->find<Piece>();

                if (piece->selectedBy == _id)
                    piece->selectedBy = -1;
            }
        }
    }

    else if (_type == UNSELECT)
        players[_id]->unselect();
}

void Client::registerEvent(sf::Int8 _type)
{
    if (_type == SHUTDOWN)
    {
        std::cout << "SHUT DOWN" << std::endl;
    }
}

void Client::emitEvent(sf::Int8 _type, sf::Int8 _id, vec2 _mp)
{
    packetSending << _type << _id << _mp.x << _mp.y;
    sendPacket();
}

void Client::receivePacket()
{
    sf::Packet event;
    auto status = socket.receive(event);
    packetReceived = false;

    if (status == sf::Socket::Disconnected)
    {
        std::cout << "Disconnected from server" << std::endl;

        for (Entity* e: Entity::findAllByTag("Player"))
            e->destroy();

        getEntity()->destroy();

        return;
    }

    else if (status == sf::Socket::Error)
        std::cout << "Error status" << std::endl;

    else if (status != sf::Socket::Done)
        std::cout << "Status unchecked: " << status << std::endl;

    else
    {
        sf::Int8 type;
        event >> type;

        std::cout << "New event: " << (int)type << std::endl;

        if (type == JOIN)
        {
            std::string playersString, piecesString;
            event >> playersString >> piecesString;

            registerEvent(type, playersString, piecesString);
        }

        else if (type == SELECT)
        {
            sf::Int8 idSrc, idDest;
            event >> idSrc >> idDest;

            registerEvent(type, idSrc, idDest);
        }

        else if (type == MOUSE)
        {
            sf::Int8 id;
            vec2 mp;
            event >> id >> mp.x >> mp.y;

            registerEvent(type, id, mp);
        }

        else
        {
            sf::Int8 id;
            event >> id;

            registerEvent(type, id);
        }
    }
}

void Client::sendPacket()
{
    socket.send(packetSending);
    packetSending.clear();
}
