#include "Connection.h"
#include "Log.h"

#include <fcntl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

using namespace std;

Connection::Connection(const int socket) {
    socket_ = socket;
    
    if (fcntl(socket_, F_SETFL, O_NONBLOCK) == -1)
        Log(WARNING) << "Could not make non-blocking sockets\n";
    
    int on = 1;
    
    if (setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&on), sizeof(on)) < 0)
        Log(WARNING) << "Could not set TCP_NODELAY\n";
}

bool Connection::operator==(const Connection &connection) {
    return socket_ == connection.socket_;
}

bool Connection::operator==(const int fd) {
    return socket_ == fd;
}

int Connection::getSocket() const {
    return socket_;
}

PartialPacket& Connection::getPartialPacket() {
    if(in_queue_.empty() || in_queue_.back().isFinished()) {
        addPartialPacket(PartialPacket());
    }
    
    return in_queue_.back();
}

void Connection::addPartialPacket(const PartialPacket &partialPacket) {
    in_queue_.push_back(partialPacket);
}

bool Connection::hasIncomingPacket() const {
    return in_queue_.empty() ? false : in_queue_.front().isFinished();
}

PartialPacket& Connection::getIncomingPacket() {
    if(in_queue_.empty()) {
        Log(ERROR) << "Trying to get an incoming packet while the inQueue is empty\n";
    }
    
    return in_queue_.front();
}

void Connection::processedPacket() {
    if(in_queue_.empty()) {
        Log(ERROR) << "Trying to pop_front when the inQueue is empty\n";
        
        return;
    }
    
    in_queue_.pop_front();
}

// TODO: Implement this on a later stage
bool Connection::isVerified() const {
    return true;
}