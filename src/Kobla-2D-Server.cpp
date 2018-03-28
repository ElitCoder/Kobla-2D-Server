#include "Log.h"
#include "Base.h"

// How long to wait before doing other server related things, in ms
#define PACKET_WAIT_TIME	(100)

using namespace std;

// TODO: Only single threaded processing/logic/disconnecting for now
mutex g_main_sync;

static void printStart() {
	Log(NONE) << "Kobla-2D-Server-Rebased [alpha] [" << __DATE__ << " @ " << __TIME__ << "]\n";
}

static void logic() {
	auto next_sync = chrono::system_clock::now();
	
	while (true) {
		if ((chrono::system_clock::now() - next_sync).count() > 0) {
			next_sync += chrono::milliseconds(PACKET_WAIT_TIME);
			
			// Handle logic
			g_main_sync.lock();
			Base::game().logic();
			g_main_sync.unlock();
		}
		
		this_thread::sleep_for(chrono::milliseconds(PACKET_WAIT_TIME / 10));
	}
}

static void process() {
	Log(DEBUG) << "Getting port information\n";
	const unsigned int port = Base::settings().get<unsigned short>("port");
	
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