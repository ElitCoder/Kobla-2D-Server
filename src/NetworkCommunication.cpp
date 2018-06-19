#include "NetworkCommunication.h"
#include "Base.h"
#include "Log.h"
#include "Connection.h"
#include "Config.h"
#include "Packet.h"
#include "PartialPacket.h"
#include "Game.h"

#include <algorithm>
#include <csignal>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>

using namespace std;

void receiveThread(NetworkCommunication &networkCommunication, int thread_id) {
    signal(SIGPIPE, SIG_IGN);
    
    fd_set readSet, errorSet;
    array<unsigned char, BUFFER_SIZE> buffer;
    
    while(true) {
        networkCommunication.setFileDescriptorsReceive(readSet, errorSet, thread_id);
        
        if(!networkCommunication.runSelectReceive(readSet, errorSet, buffer.data(), thread_id)) {
            break;
        }
    }
    
    Log(ERROR) << "Terminating receiveThread\n";
}

vector<string> NetworkCommunication::getStats() {
    for (auto& lock : mConnectionsMutex)
        lock->lock();

    for (auto& lock : mOutgoingMutex)
        lock->lock();
        
    lock_guard<mutex> lock(mIncomingMutex);
    
    string connections =    "Number of connections: ";
    string incoming =       "Number of incoming packets in queue: " + to_string(mIncomingPackets.size());
    string outgoing =       "Number of outgoing packets in queue: ";
    
    for (auto& queue : mOutgoingPackets)
        outgoing +=         "(" + to_string(queue.size()) + ") ";

    for (auto& queue : mConnections)
        connections +=      "(" + to_string(queue.size()) + ")";
    
    vector<string> lines = { connections, incoming, outgoing };
    
    for (auto& queue : mConnections) {
        string values = "";
        
        for (auto& peer : queue) {
            auto& connection = peer.second;
            
            values += to_string(connection.getSocket()) + ": " + to_string(connection.packetsWaiting()) + " ";
        }
        
        lines.push_back(values);
    }
    
    for (auto& lock : mOutgoingMutex)
        lock->unlock();
            
    for (auto& lock : mConnectionsMutex)
        lock->unlock();
    
    return lines;
}

static void statsThread(NetworkCommunication& network) {
    if (!Base::settings().get<bool>("stats", false))
        return;
        
    auto delay = Base::settings().get<int>("stats_delay", 3000);
    auto next_sync = chrono::system_clock::now() + chrono::milliseconds(delay);
    
    while (true) {
        if ((chrono::system_clock::now() - next_sync).count() > 0) {
            auto stats = network.getStats();
            
            Log(INFORMATION) << "Network stats:\n";
            
            for (auto& line : stats)
                Log(INFORMATION) << line << endl;
            
            next_sync += chrono::milliseconds(delay);
		}
        
        this_thread::sleep_for(chrono::milliseconds(delay / 10));
    }
}

void sendThread(NetworkCommunication &networkCommunication, int thread_id) {
    signal(SIGPIPE, SIG_IGN);

    while(true) {
        auto& packet = networkCommunication.waitForOutgoingPackets(thread_id);
        
        int sending = min((unsigned int)NetworkConstants::BUFFER_SIZE, packet.second.getSize() - packet.second.getSent());
        int sent = send(packet.first, packet.second.getData() + packet.second.getSent(), sending, 0);
        
        bool removePacket = false;
        
        if(sent <= 0) {
            if(sent == 0) {
                removePacket = true;
            }
            
            else {
                if(errno == EWOULDBLOCK || errno == EAGAIN) {
                    continue;
                }
                
                else {
                    removePacket = true;
                }
            }
        }
        
        else {
            packet.second.addSent(sent);
            
            if(packet.second.fullySent()) {
                removePacket = true;
            }
        }
        
        if(removePacket)
            networkCommunication.removeOutgoingPacket(thread_id);
    }
    
    Log(ERROR) << "Terminating sendThread\n";
}

void acceptThread(NetworkCommunication &networkCommunication) {
    signal(SIGPIPE, SIG_IGN);

    fd_set readSet, errorSet;
    
    while(true) {
        networkCommunication.setFileDescriptorsAccept(readSet, errorSet);
        
        if(!networkCommunication.runSelectAccept(readSet, errorSet)) {
            break;
        }
    }
    
    Log(ERROR) << "Terminating acceptThread\n";
}

bool setupServer(int &masterSocket, const unsigned short port) {
    signal(SIGPIPE, SIG_IGN);
    
    masterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(masterSocket < 0) {
        Log(ERROR) << "socket() failed\n";
        
        return false;
    }
    
    int on = 1;
    
    if(setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&on), sizeof(on)) < 0) {
        Log(ERROR) << "Not reusable address\n";
        
        return false;
    }
    
    sockaddr_in socketInformation;
    socketInformation.sin_family = AF_INET;
    socketInformation.sin_addr.s_addr = INADDR_ANY;
    socketInformation.sin_port = htons(port);
    
    if(bind(masterSocket, reinterpret_cast<sockaddr*>(&socketInformation), sizeof(socketInformation)) < 0) {
        Log(ERROR) << "bind() failed\n";
        
        close(masterSocket);
        return false;
    }
    
    size_t socketInformationSize = sizeof(socketInformation);

	if(getsockname(masterSocket, reinterpret_cast<sockaddr*>(&socketInformation), reinterpret_cast<socklen_t*>(&socketInformationSize)) < 0) {
        Log(ERROR) << "Could not get address information\n";
        
        close(masterSocket);
        return false;
	}
	
	listen(masterSocket, 16);
    
    return true;
}

unsigned int NetworkCommunication::processBuffer(const unsigned char *buffer, const unsigned int received, PartialPacket &partialPacket) {
    if(partialPacket.hasHeader()) {        
        unsigned int insert = received;
        
        if(partialPacket.getSize() + received >= partialPacket.getFullSize()) {            
            insert = partialPacket.getFullSize() - partialPacket.getSize();
        }
        
        if(insert > received) {
            Log(ERROR) << "Trying to insert more data than available in buffer, insert = " << insert << ", received = " << received << endl;
        }
        
        partialPacket.addData(buffer, insert);
                
        return insert;
    }
    
    else {
        unsigned int leftHeader = 4 - partialPacket.getSize();
        unsigned int adding = received > leftHeader ? leftHeader : received;
        
        if(adding > received) {
            Log(ERROR) << "Trying to insert more data than available in buffer, adding = " << adding << ", received = " << received << endl;
        }
        
        partialPacket.addData(buffer, adding);
        
        return adding;
    }
}

void NetworkCommunication::assemblePacket(const unsigned char *buffer, const unsigned int received, Connection &connection) {      
    unsigned int processed = 0;
    
    do {
        processed += processBuffer(buffer + processed, received - processed, connection.getPartialPacket());
    } while(processed < received);
        
    moveProcessedPacketsToQueue(connection);
}

void NetworkCommunication::moveProcessedPacketsToQueue(Connection &connection) {
    if(!connection.hasIncomingPacket()) {
        return;
    }
    
    lock_guard<mutex> guard(mIncomingMutex);
    
    while(connection.hasIncomingPacket()) {
        mIncomingPackets.push_back({ connection.getSocket(), connection.getUniqueID(), move(connection.getIncomingPacket()) });
        connection.processedPacket();
    }
    
    mIncomingCV.notify_one();
}

NetworkCommunication::NetworkCommunication() {}

NetworkCommunication::~NetworkCommunication() {
    // Disconnect Clients
    for (auto& queue : mConnections) {
        for (auto& connection_peer : queue)
            close(connection_peer.second.getSocket());
    }
}

// Same as old constructor
void NetworkCommunication::start(unsigned short port, int num_sending_threads, int num_receiving_threads) {
    num_sending_threads_ = num_sending_threads;
    num_receiving_threads_ = num_receiving_threads;
    
    if(!setupServer(mSocket, port)) {
        return;
    }

    for (int i = 0; i < num_sending_threads; i++) {
        mOutgoingCV.push_back(make_shared<condition_variable>());
        mOutgoingMutex.push_back(make_shared<mutex>());
        mOutgoingPackets.emplace_back();
    }
    
    mPipe.resize(num_receiving_threads);
    mConnections.resize(num_receiving_threads);
    
    for (int i = 0; i < num_receiving_threads; i++)
        mConnectionsMutex.push_back(make_shared<mutex>());
    
    mAcceptThread = thread(acceptThread, ref(*this));
    mStatsThread = thread(statsThread, ref(*this));
    
    mSendThread.resize(num_sending_threads);
    
    for (int i = 0; i < num_sending_threads; i++)
        mSendThread.at(i) = thread(sendThread, ref(*this), i);
        
    mReceiveThread.resize(num_receiving_threads);
    
    for (int i = 0; i < num_receiving_threads; i++)
        mReceiveThread.at(i) = thread(receiveThread, ref(*this), i);
}

void NetworkCommunication::setFileDescriptorsAccept(fd_set &readSet, fd_set &errorSet) {
    FD_ZERO(&readSet);
    FD_ZERO(&errorSet);
    
    FD_SET(mSocket, &readSet);
    FD_SET(mSocket, &errorSet);
}

void NetworkCommunication::setFileDescriptorsReceive(fd_set &readSet, fd_set &errorSet, int thread_id) {
    FD_ZERO(&readSet);
    FD_ZERO(&errorSet);
    
    FD_SET(mPipe.at(thread_id).getSocket(), &readSet);
    FD_SET(mPipe.at(thread_id).getSocket(), &errorSet);
    
    lock_guard<mutex> guard(*mConnectionsMutex.at(thread_id));
    
    for_each(mConnections.at(thread_id).begin(), mConnections.at(thread_id).end(), [&readSet, &errorSet] (auto& connectionPair) {
        FD_SET(connectionPair.second.getSocket(), &readSet);
        FD_SET(connectionPair.second.getSocket(), &errorSet);
    });
}

bool NetworkCommunication::runSelectReceive(fd_set &readSet, fd_set &errorSet, unsigned char *buffer, int thread_id) {    
    if(select(FD_SETSIZE, &readSet, NULL, &errorSet, NULL) == 0) {
        Log(ERROR) << "select() returned 0 when there's no timeout\n";
        
        return false;
    }
    
    if(FD_ISSET(mPipe.at(thread_id).getSocket(), &errorSet)) {
        Log(ERROR) << "errorSet was set in pipe, errno = " << errno << '\n';
        
        return false;
    }
    
    if(FD_ISSET(mPipe.at(thread_id).getSocket(), &readSet)) {
        Log(DEBUG) << "Pipe was activated\n";
        
        mPipe.at(thread_id).resetPipe();
        return true;
    }
        
    lock_guard<mutex> guard(*mConnectionsMutex.at(thread_id));
    
    for(auto& connection_peer : mConnections.at(thread_id)) {
        bool removeConnection = false;
        auto& connection = connection_peer.second;
        
        if (connection.packetsWaiting() > NetworkConstants::MAX_WAITING_PACKETS_PER_CLIENT)
            continue;
        
        if(FD_ISSET(connection.getSocket(), &errorSet)) {
            Log(WARNING) << "errorSet was set in connection\n";
            
            removeConnection = true;
        }
        
        else if(FD_ISSET(connection.getSocket(), &readSet)) {
            int received = recv(connection.getSocket(), buffer, BUFFER_SIZE, 0);
            
            if(received <= 0) {
                if(received == 0) {
                    Log(INFORMATION) << "Connection #" << connection.getUniqueID() << " disconnected\n";
                    
                    removeConnection = true;
                }
                
                else {
                    if(errno == EWOULDBLOCK || errno == EAGAIN) {
                        Log(WARNING) << "EWOULDBLOCK activated\n";
                    }
                    
                    else {
                        Log(WARNING) << "Connection #" << connection.getUniqueID() << " failed with errno = " << errno << '\n';
                        
                        removeConnection = true;
                    }
                }
            }
            
            else {
                assemblePacket(buffer, received, connection);
            }
        }
        
        if(removeConnection) {
            // Remove from game
            Base::game().disconnected(connection);
            
            if (close(connection.getSocket()) < 0)
                Log(ERROR) << "close() got errno = " << errno << endl;
            
            mConnections.at(thread_id).erase(connection_peer.first);
            
            break;
        }
    }
        
    return true;
}

void NetworkCommunication::sendToAllExcept(const Packet &packet, const std::vector<int> &except) {
    // Go through all receiving threads to find the Clients
    for (size_t i = 0; i < mConnections.size(); i++) {
        lock_guard<mutex> lock(*mConnectionsMutex.at(i));
        auto& connections = mConnections.at(i);
        
        for (auto& connection_peer : connections) {
            if (!except.empty())
                if (find_if(except.begin(), except.end(), [&connection_peer] (auto fd) { return connection_peer.second == fd; }) != except.end())
                    continue;
            
            addOutgoingPacket(connection_peer.second.getSocket(), packet);
        }
    }
}

void NetworkCommunication::sendToAll(const Packet& packet) {
    sendToAllExcept(packet, {});
}

bool NetworkCommunication::runSelectAccept(fd_set &readSet, fd_set &errorSet) {
    if(select(FD_SETSIZE, &readSet, NULL, &errorSet, NULL) == 0) {
        Log(ERROR) << "select() returned 0 when there's no timeout\n";
        
        return false;
    }
    
    if(FD_ISSET(mSocket, &errorSet)) {
        Log(ERROR) << "masterSocket received an error\n";
        
        return false;
    }
    
    if(FD_ISSET(mSocket, &readSet)) {
        int newSocket = accept(mSocket, 0, 0);
        
        if(newSocket < 0) {
            Log(ERROR) << "accept() failed with " << errno << endl;
            
            return false;
        }
        
        int index = newSocket % num_receiving_threads_;
        
        {
            lock_guard<mutex> guard(*mConnectionsMutex.at(index));
            
            Connection connection(newSocket);
            mConnections.at(index).insert({ connection.getUniqueID(), connection });
            
            Log(INFORMATION) << "Connection #" << connection.getUniqueID() << " added\n";
            Log(DEBUG) << "Socket ID " << connection.getSocket() << endl;
        }
        
        mPipe.at(index).setPipe();
    }
    
    return true;
}

int NetworkCommunication::getSocket() const {
    return mSocket;
}

pair<int, Packet>& NetworkCommunication::waitForOutgoingPackets(int thread_id) {
    unique_lock<mutex> lock(*mOutgoingMutex.at(thread_id));
    mOutgoingCV.at(thread_id)->wait(lock, [this, &thread_id] { return !mOutgoingPackets.at(thread_id).empty(); });
    
    return mOutgoingPackets.at(thread_id).front();
}

void NetworkCommunication::addOutgoingPacket(const int fd, const Packet &packet) {
    auto index = fd % num_sending_threads_;
        
    lock_guard<mutex> guard(*mOutgoingMutex.at(index));
    mOutgoingPackets.at(index).push_back({ fd, packet });
    mOutgoingCV.at(index)->notify_one();
}

void NetworkCommunication::send(int fd, const Packet& packet) {
    addOutgoingPacket(fd, packet);
}

int NetworkCommunication::getConnectionSocket(size_t unique_id) {
    for (size_t i = 0; i < mConnections.size(); i++) {
        lock_guard<mutex> lock(*mConnectionsMutex.at(i));
        auto& queue = mConnections.at(i);
        
        auto iterator = queue.find(unique_id);
        
        if (iterator != queue.end())
            return iterator->second.getSocket();
    }
    
    return -1;
}

void NetworkCommunication::sendUnique(size_t id, const Packet& packet) {
    auto fd = getConnectionSocket(id);
    
    if (fd < 0) {
        Log(WARNING) << "Could not find connection with unique ID " << id << endl;
        
        return;
    }
        
    addOutgoingPacket(fd, packet);
}

void NetworkCommunication::removeOutgoingPacket(int thread_id) {
    lock_guard<mutex> guard(*mOutgoingMutex.at(thread_id));
    
    if(mOutgoingPackets.at(thread_id).empty()) {
        Log(ERROR) << "Trying to pop when outgoingPackets is empty\n";
        
        return;
    }
    
    mOutgoingPackets.at(thread_id).pop_front();
}

tuple<int, size_t, Packet> NetworkCommunication::waitForProcessingPackets() {
    tuple<int, size_t, Packet> packet;
    
    {
        unique_lock<mutex> lock(mIncomingMutex);
        mIncomingCV.wait(lock, [this] { return !mIncomingPackets.empty(); });
        
        // Grab Packet
        packet = mIncomingPackets.front();
        mIncomingPackets.pop_front();
    }
    
    // Set Connection to successfully processed Packet thus decreasing waiting Packets counter
    auto id = get<1>(packet);
    
    for (size_t i = 0; i < mConnections.size(); i++) {
        lock_guard<mutex> lock(*mConnectionsMutex.at(i));
        auto& queue = mConnections.at(i);
        
        auto iterator = queue.find(id);
        
        if (iterator == queue.end())
            continue;
            
        iterator->second.reducePacketsWaiting();
    }
    
    return packet;
}

/*
    EventPipe
*/

EventPipe::EventPipe() {
    event_mutex_ = make_shared<mutex>();
    
    if(pipe(mPipes) < 0) {
        Log(ERROR) << "Failed to create pipe, won't be able to wake threads, errno = " << errno << '\n';
    }
    
    if(fcntl(mPipes[0], F_SETFL, O_NONBLOCK) < 0) {
        Log(WARNING) << "Failed to set pipe non-blocking mode\n";
    }
}

EventPipe::~EventPipe() {
    if (mPipes[0] >= 0)
        close(mPipes[0]);
    
    if (mPipes[1] >= 0)
        close(mPipes[1]);
}

void EventPipe::setPipe() {
    lock_guard<mutex> lock(*event_mutex_);
    
    if (write(mPipes[1], "0", 1) < 0)
        Log(ERROR) << "Could not write to pipe, errno = " << errno << '\n';
}

void EventPipe::resetPipe() {
    lock_guard<mutex> lock(*event_mutex_);
    
    unsigned char buffer;

    while(read(mPipes[0], &buffer, 1) == 1)
        ;
}

int EventPipe::getSocket() {
    lock_guard<mutex> lock(*event_mutex_);
    
    return mPipes[0];
}