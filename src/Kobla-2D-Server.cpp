#include "Log.h"
#include "Base.h"

// How long to wait before doing other server related things, in ms
#define PACKET_WAIT_TIME	(50)

using namespace std;

void process() {
	Log(DEBUG) << "Getting port information\n";
	const unsigned int port = Base::settings().get<unsigned short>("port");
	
	Log(DEBUG) << "Starting network\n";
	Base::network().start(port, PACKET_WAIT_TIME);
}

int main() {
	Log(DEBUG) << "Parsing config\n";
	Base::settings().parse("config");
	
	Log(DEBUG) << "Processing\n";
	process();
	
	return 0;
}