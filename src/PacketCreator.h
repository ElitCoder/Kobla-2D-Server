#ifndef PACKET_CREATOR_H
#define PACKET_CREATOR_H

#include "Packet.h"

enum {
	HEADER_LOGIN,
	HEADER_GET_CHARACTERS
};

class PacketCreator {
public:
	static Packet answerLogin(bool success);
};

#endif