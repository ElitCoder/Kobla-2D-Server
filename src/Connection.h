#ifndef CONNECTION_H
#define CONNECTION_H

#include "Packet.h"
#include "PartialPacket.h"

#include <list>

class Connection {
public:
    Connection(const int socket);
    
    bool operator==(const Connection &connection);
    bool operator==(const int fd);

    int getSocket() const;
    PartialPacket& getPartialPacket();
    void addPartialPacket(const PartialPacket &partialPacket);
    bool hasIncomingPacket() const;
    PartialPacket& getIncomingPacket();
    void processedPacket();
    
    bool isVerified() const;
    
private:
    int socket_;
    
    std::list<PartialPacket> in_queue_;
};

#endif