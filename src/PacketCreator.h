#ifndef PACKET_CREATOR_H
#define PACKET_CREATOR_H

#include "Packet.h"

enum {
	HEADER_LOGIN,
	HEADER_GET_CHARACTERS,
	HEADER_PING_ALIVE,
	HEADER_UNKNOWN
};

class PacketCreator {
public:
	static Packet answerLogin(bool success);
	static Packet unknown();
};

#endif