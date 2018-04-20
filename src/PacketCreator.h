#ifndef PACKET_CREATOR_H
#define PACKET_CREATOR_H

#include "Packet.h"
#include "Character.h"
#include "Player.h"
#include "TemporaryObject.h"

enum {
	HEADER_LOGIN,
	HEADER_GET_CHARACTERS,
	HEADER_PING_ALIVE,
	HEADER_UNKNOWN,
	HEADER_SPAWN,
	HEADER_MOVE,
	HEADER_ADD_PLAYER,
	HEADER_REMOVE_CHARACTER,
	HEADER_UPDATE_HEALTH,
	HEADER_SHOOT
};

class PacketCreator {
public:
	static Packet answerLogin(bool success);
	static Packet unknown();
	static Packet spawn(const Player& player);
	static Packet move(const Character* character);
	static Packet addPlayer(const Character* character);
	static Packet remove(const Character* character);
	static Packet health(const Character* character);
	static Packet shoot(const TemporaryObject& bullet);
};

#endif