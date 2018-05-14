#include "PacketCreator.h"
#include "Packet.h"
#include "Player.h"
#include "TemporaryObject.h"

using namespace std;

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

static void addCollisionInformation(Packet& packet, const array<bool, COLLISION_MAX>& collisions) {
	packet.addInt(collisions.size());
	
	for (const auto& collision : collisions)
		packet.addBool(collision);
}

// Adding the same things anyway
static void playerAddInformation(Packet& packet, const Character* player) {
	packet.addInt(player->getID());
	packet.addInt(player->getObjectID());
	packet.addFloat(player->getX());
	packet.addFloat(player->getY());
	packet.addString(player->getName());
	packet.addFloat(player->getMovingSpeed());
	
	// Add all collision information
	addCollisionInformation(packet, player->getCollisions());
	
	packet.addBool(player->isColliding());
	packet.addFloat(player->getFullHealth());
	packet.addFloat(player->getCurrentHealth());
	packet.addInt(player->getObjectType());
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
	
	auto destination = player->getDeterminedDestination();
	
	packet.addBool(player->hasDeterminedDestination());
	packet.addFloat(destination.first);
	packet.addFloat(destination.second);
	
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
	
	auto destination = character->getDeterminedDestination();
	
	packet.addBool(character->hasDeterminedDestination());
	packet.addFloat(destination.first);
	packet.addFloat(destination.second);
	
	packet.finalize();
	
	return packet;
}

Packet PacketCreator::remove(const Object* object) {
	Packet packet;
	packet.addHeader(HEADER_REMOVE_CHARACTER);
	packet.addInt(object->getID());
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
	packet.addInt(bullet.getOwner());
	
	addCollisionInformation(packet, bullet.getCollisions());
	
	packet.finalize();
	
	return packet;
}

Packet PacketCreator::text(const Object* object, const string& output, int ms) {
	Packet packet;
	packet.addHeader(HEADER_TEXT);
	packet.addString(output);
	packet.addInt(ms);
	packet.addInt(object->getID());
	packet.finalize();
	
	return packet;
}