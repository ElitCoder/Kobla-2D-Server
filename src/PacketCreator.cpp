#include "PacketCreator.h"
#include "Packet.h"
#include "Player.h"
#include "TemporaryObject.h"
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
static void playerAddInformation(Packet& packet, const Character* player) {
	packet.addInt(player->getID());
	packet.addInt(player->getObjectID());
	packet.addFloat(player->getX());
	packet.addFloat(player->getY());
	packet.addString(player->getName());
	packet.addFloat(player->getMovingSpeed());
	packet.addBool(player->getCollision());
	packet.addFloat(player->getFullHealth());
	packet.addFloat(player->getCurrentHealth());
}

Packet PacketCreator::spawn(const Player& player) {
	Packet packet;
	packet.addHeader(HEADER_SPAWN);
	packet.addInt(player.getMapID());
	
	playerAddInformation(packet, &player);
	
	packet.finalize();
	
	return packet;
}

Packet PacketCreator::addPlayer(const Character* player) {
	Packet packet;
	packet.addHeader(HEADER_ADD_PLAYER);
	packet.addBool(player->isMoving());
	packet.addInt(player->getMovingDirection());
	
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
	packet.addFloat(character->getPredeterminedDistance());
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

Packet PacketCreator::health(const Character* character) {
	Packet packet;
	packet.addHeader(HEADER_UPDATE_HEALTH);
	packet.addInt(character->getID());
	packet.addFloat(character->getFullHealth());
	packet.addFloat(character->getCurrentHealth());
	packet.finalize();
	
	return packet;
}

Packet PacketCreator::shoot(const TemporaryObject& bullet) {
	Packet packet;
	packet.addHeader(HEADER_SHOOT);
	packet.addInt(bullet.getMovingDirection());
	packet.addFloat(bullet.getMovingSpeed());
	packet.addInt(bullet.getType());
	packet.addInt(bullet.getID());
	packet.addFloat(bullet.getX());
	packet.addFloat(bullet.getY());
	packet.finalize();
	
	return packet;
}