#ifndef CONNECTION_H
#define CONNECTION_H

#include "Packet.h"
#include "PartialPacket.h"
//#include "Player.h"

#include <list>

class Connection {
public:
    Connection(const int socket);
    
    Connection(const Connection &connection) = delete;
    Connection(Connection &&connection);// = delete;
    
    Connection& operator=(const Connection &connection) = delete;
    Connection& operator=(Connection &&connection);// = delete;
    
    bool operator==(const Connection &connection);
    bool operator==(const int fd);

    int getSocket() const;
    //void sendPacket(const Packet &packet);
    //const Packet& nextPacket() const;
    //bool hasOutQueue() const;
    //void successfullySent(const int sent);
    PartialPacket& getPartialPacket();
    void addPartialPacket(const PartialPacket &partialPacket);
    bool hasIncomingPacket() const;
    PartialPacket& getIncomingPacket();
    void processedPacket();
    
    //void setPlayer(Player *player);
    //Player* getPlayer();
    
private:
    int m_socket;
    
    //std::list<Packet> m_outQueue;
    std::list<PartialPacket> m_inQueue;
    
    //Player *mPlayer;
};

#endif