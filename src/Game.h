#ifndef GAME_H
#define GAME_H

#include "Connection.h"
#include "Packet.h"
#include "Player.h"

class Game {
public:
	Game();
	
	void process(Connection& connection, Packet& packet);
	void logic();
	
private:
	Player* getPlayer(const Connection& connection);
	
	void handleLogin();
	void handleGetCharacters();
	
	std::vector<Player> players_;
	
	Player* current_player_;
	Connection* current_connection_;
	Packet* current_packet_;
};

#endif