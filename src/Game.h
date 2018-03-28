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
	
	void disconnected(const Connection& connection);
	
private:
	void addPlayer(const Player& player);
	Player* getPlayer(const Connection& connection);
	std::vector<Player*> getPlayersOnMap(const std::vector<Player*>& except, int map_id);
	
	void handleLogin();
	void handleGetCharacters();
	void handleUnknownPacket();
	void handleSpawn();
	void handleMove();
	
	std::vector<Player> players_;
	
	Player* current_player_;
	Connection* current_connection_;
	Packet* current_packet_;
};

#endif