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

// Adding the same things anyway
static void playerAddInformation(Packet& packet, const Player& player) {
	packet.addInt(player.getTextureID());
	packet.addFloat(player.getX());
	packet.addFloat(player.getY());
	packet.addString(player.getName());
	packet.addFloat(player.getMovingSpeed());
}

Packet PacketCreator::spawn(const Player& player) {
	Packet packet;
	packet.addHeader(HEADER_SPAWN);
	packet.addInt(player.getMapID());
	
	playerAddInformation(packet, player);
	
	packet.finalize();
	
	return packet;
}

Packet PacketCreator::addPlayer(const Player& player) {
	Packet packet;
	packet.addHeader(HEADER_ADD_PLAYER);
	packet.addInt(player.getID());
	packet.addBool(player.isMoving());
	packet.addInt(player.getMovingDirection());
	
	playerAddInformation(packet, player);

	packet.finalize();
	
	return packet;
}

Packet PacketCreator::move(const Character* character) {
	Packet packet;
	packet.addHeader(HEADER_MOVE);
	packet.addBool(character->isMoving());
	packet.addFloat(character->getX());
	packet.addFloat(character->getY());
	packet.addInt(character->getMovingDirection());
	packet.addInt(character->getID());
	packet.finalize();
	
	return packet;
}

Packet PacketCreator::remove(const Character* character) {
	Packet packet;
	packet.addHeader(HEADER_REMOVE_CHARACTER);
	packet.addInt(character->getID());
	packet.finalize();
	
	return packet;
}