#include "Log.h"
#include "Base.h"

// How long to wait before doing other server related things, in ms
#define PACKET_WAIT_TIME	(100)

using namespace std;

static void printStart() {
	Log(NONE) << "Kobla-2D-Server-Rebased [alpha] [" << __DATE__ << " @ " << __TIME__ << "]\n";
}

static void process() {
	Log(DEBUG) << "Getting port information\n";
	const unsigned int port = Base::settings().get<unsigned short>("port");
	
	Log(DEBUG) << "Starting network\n";
	Base::network().start(port, PACKET_WAIT_TIME);
	
	printStart();
	
	auto next_sync = chrono::system_clock::now();
	
	while (true) {
		auto* fd_packet = Base::network().waitForProcessingPackets();
		
		if (fd_packet != nullptr) {
			auto* connection_pair = Base::network().getConnectionAndLock(fd_packet->first);
			
			// TODO: Rewrite this so the sync timer is not skipped
			if (connection_pair == nullptr) {
				Base::network().removeProcessingPacket();
				
				continue;
			}
			
			// Handle packet
			Base::game().process(connection_pair->second, fd_packet->second);
			
			// Remove packet from processing queue
			Base::network().unlockConnection(*connection_pair);
			Base::network().removeProcessingPacket();
		}
		
		if ((chrono::system_clock::now() - next_sync).count() > 0) {
			next_sync += chrono::milliseconds(PACKET_WAIT_TIME);
			
			// Handle logic
			Base::game().logic();
		}
	}
}

int main() {
	Log(DEBUG) << "Parsing config\n";
	Base::settings().parse("config");
	
	Log(DEBUG) << "Processing\n";
	process();
	
	return 0;
}