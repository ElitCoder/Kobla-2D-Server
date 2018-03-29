#ifndef GAME_H
#define GAME_H

#include "Connection.h"
#include "Packet.h"
#include "Player.h"
#include "NPC.h"

class Game {
public:
	Game();
	
	void process(Connection& connection, Packet& packet);
	void logic();
	void load();
	
	void disconnected(const Connection& connection);
	
private:
	void parseNPCs(const std::vector<std::pair<int, std::deque<std::string>>>& npcs);
	void parseMaps(const std::vector<std::pair<int, std::deque<std::string>>>& maps);
	
	NPC* getReferenceNPC(int id);
	std::vector<NPC*> getNPCsOnMap(int map_id);
	
	void addPlayer(const Player& player);
	Player* getPlayer(const Connection& connection);
	std::vector<Player*> getPlayersOnMap(const std::vector<Player*>& except, int map_id);
	
	void handleLogin();
	void handleGetCharacters();
	void handleUnknownPacket();
	void handleSpawn();
	void handleMove();
	
	std::vector<Player> players_;
	std::vector<NPC> npcs_;
	std::vector<NPC> reference_npcs_;
	
	Player* current_player_;
	Connection* current_connection_;
	Packet* current_packet_;
};

#endif