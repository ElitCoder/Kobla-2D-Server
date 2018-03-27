#include "PacketCreator.h"

Packet PacketCreator::answerLogin(bool success) {
	Packet packet;
	packet.addHeader(HEADER_LOGIN);
	packet.addBool(success);
	packet.finalize();
	
	return packet;
}

Packet PacketCreator::unknown() {
	Packet packet;
	packet.addHeader(HEADER_UNKNOWN);
	packet.finalize();
	
	return packet;
}

Packet PacketCreator::spawn() {
	Packet packet;
	packet.addHeader(HEADER_SPAWN);
	
	// Map ID
	packet.addInt(0);
	
	// Player texture ID
	packet.addInt(0);
	
	// Player position x, y
	packet.addInt(100);
	packet.addInt(100);
	
	// Player name
	packet.addString("Igge");
	
	packet.finalize();
	
	return packet;
}