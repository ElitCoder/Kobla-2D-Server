#include "Connection.h"

#include <fcntl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;

Connection::Connection(const int socket) : m_socket(socket) {
    if(fcntl(m_socket, F_SETFL, O_NONBLOCK) == -1) {
        cout << "WARNING: could not make non-blocking sockets\n";
    }
    
    int on = 1;
    
    if(setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&on), sizeof(on)) < 0) {
        cout << "WARNING: could not set TCP_NODELAY\n";
    }
}

Connection& Connection::operator=(Connection &&connection) {
    m_socket = connection.m_socket;
    connection.m_socket = -1;
    
    //mPlayer = connection.mPlayer;
    //connection.mPlayer = nullptr;
    
    // Should not be needed
    m_inQueue = move(connection.m_inQueue);
    //m_outQueue = move(connection.m_outQueue);
    
    return *this;
}

Connection::Connection(Connection &&connection) {
    m_socket = connection.m_socket;
    connection.m_socket = -1;
    
    //mPlayer = connection.mPlayer;
    //connection.mPlayer = nullptr;
    
    // Should not be needed
    m_inQueue = move(connection.m_inQueue);
    //m_outQueue = move(connection.m_outQueue);
}

bool Connection::operator==(const Connection &connection) {
    return m_socket == connection.m_socket;
}

bool Connection::operator==(const int fd) {
    return m_socket == fd;
}

int Connection::getSocket() const {
    return m_socket;
}

/*
void Connection::sendPacket(const Packet &packet) {
    m_outQueue.push_back(packet);
}

const Packet& Connection::nextPacket() const {
    return m_outQueue.front();
}

bool Connection::hasOutQueue() const {
    return !m_outQueue.empty();
}

void Connection::successfullySent(const int sent) {
    Packet &packet = m_outQueue.front();
    
    if(sent + packet.getSent() >= packet.getSize()) {
        m_outQueue.pop_front();
    }
    
    else {
        packet.addSent(sent);
    }
}
*/

PartialPacket& Connection::getPartialPacket() {
    if(m_inQueue.empty() || m_inQueue.back().isFinished()) {
        addPartialPacket(PartialPacket());
    }
    
    return m_inQueue.back();
}

void Connection::addPartialPacket(const PartialPacket &partialPacket) {
    m_inQueue.push_back(partialPacket);
}

bool Connection::hasIncomingPacket() const {
    return m_inQueue.empty() ? false : m_inQueue.front().isFinished();
}

PartialPacket& Connection::getIncomingPacket() {
    if(m_inQueue.empty()) {
        cout << "ERROR: trying to get an incoming packet while the inQueue is empty\n";
    }
    
    return m_inQueue.front();
}

void Connection::processedPacket() {
    if(m_inQueue.empty()) {
        cout << "ERROR: trying to pop_front when the inQueue is empty\n";
        
        return;
    }
    
    m_inQueue.pop_front();
}

/*
void Connection::setPlayer(Player *player) {
    if(player == nullptr) {
        cout << "WARNING: connection set player as nullptr\n";
    }
    
    mPlayer = player;
}

Player* Connection::getPlayer() {
    return mPlayer;
}
*/