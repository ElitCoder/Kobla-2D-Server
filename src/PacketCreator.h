#pragma once
#ifndef PACKET_CREATOR_H
#define PACKET_CREATOR_H

#include <string>

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
	HEADER_SHOOT,
	HEADER_HIT,
	HEADER_ACTIVATE,
	HEADER_TEXT,
	HEADER_CHAT
};

class Packet;
class Player;
class Character;
class TemporaryObject;
class Object;

class PacketCreator {
public:
	static Packet answerLogin(bool success);
	static Packet unknown();
	static Packet spawn(const Player& player);
	static Packet move(const Character* character);
	static Packet addPlayer(const Character* character);
	static Packet remove(const Object* object);
	static Packet health(const Character* character);
	static Packet shoot(const TemporaryObject& bullet);
	static Packet text(const Object* object, const std::string& output, int ms);
};

#endif