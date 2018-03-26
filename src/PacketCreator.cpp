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
	// Hard-coded map-id
	packet.addInt(0);
	packet.finalize();
	
	return packet;
}