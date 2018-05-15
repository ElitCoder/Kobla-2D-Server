#include "Log.h"
#include "Base.h"
#include "Game.h"
#include "Database.h"
#include "NetworkCommunication.h"
#include "Config.h"
#include "Packet.h"
#include "Connection.h"

#include <thread>

// How long to wait before doing other server related things, in ms
// Too high values will make monsters move at the same time and make collision not working properly
// Too low values will put strain on the Server
#define PACKET_WAIT_TIME	(100)

using namespace std;

// TODO: Only single threaded processing/logic/disconnecting for now
mutex g_main_sync;

static void printStart() {
	Log(NONE) << "Kobla-2D-Server-Rebased [alpha] [" << __DATE__ << " @ " << __TIME__ << "]\n";
}

static void logic() {
	Timer next_sync;
	
	while (true) {
		if (next_sync.elapsed()) {
			next_sync.start(PACKET_WAIT_TIME);
			
			// Handle logic
			g_main_sync.lock();
			Base::game().logic();
			g_main_sync.unlock();
		} else {
			this_thread::sleep_for(chrono::milliseconds(1));
		}
	}
}

static void process() {
	const unsigned int STANDARD_PORT = 11000;
	
	Log(DEBUG) << "Getting port information\n";
	const unsigned int port = Base::settings().get<unsigned short>("port", STANDARD_PORT);
	
	// Create the database (using file-based for now)
	Base::createDatabase(DATABASE_TYPE_FILE);
	
	// Load game information before starting network
	Base::game().load();
	
	Log(DEBUG) << "Starting network\n";
	Base::network().start(port, PACKET_WAIT_TIME);
	
	thread sync_thread(logic);

	while (true) {
		auto& fd_packet = Base::network().waitForProcessingPackets();
		auto* connection_pair = Base::network().getConnectionAndLock(fd_packet.first);
		
		if (connection_pair == nullptr) {
			Base::network().removeProcessingPacket();
			
			continue;
		}
		
		// Handle packet
		g_main_sync.lock();
		Base::game().process(connection_pair->second, fd_packet.second);
		g_main_sync.unlock();
		
		// Remove packet from processing queue
		connection_pair->second.finishRealProcessing();
		Base::network().unlockConnection(*connection_pair);
		Base::network().removeProcessingPacket();
	}
	
	// Shutdown
	Base::destroyDatabase();
	
	sync_thread.detach();
}

int main() {
	printStart();
	
	Log(DEBUG) << "Parsing config\n";
	Base::settings().parse("config");
	
	Log(DEBUG) << "Processing\n";
	process();
	
	return 0;
}