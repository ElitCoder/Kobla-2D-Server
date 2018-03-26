#ifndef CONNECTION_H
#define CONNECTION_H

#include "Packet.h"
#include "PartialPacket.h"

#include <deque>
#include <mutex>

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
    
    size_t waitingForRealProcessing();
    void finishRealProcessing();
    void addRealProcessing();
    
private:
    int socket_;
    std::deque<PartialPacket> in_queue_;
    
    size_t waiting_processing_;
    // TODO: Change this, will be slowdowns with multiple connections
    static std::mutex waiting_processing_mutex_;
};

#endif