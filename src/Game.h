#ifndef GAME_H
#define GAME_H

#include "Connection.h"
#include "Packet.h"

class Game {
public:
	Game();
	
	void process(Connection& connection, Packet& packet);
	void logic();
	
private:
};

#endif