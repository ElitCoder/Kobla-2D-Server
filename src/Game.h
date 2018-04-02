#ifndef GAME_H
#define GAME_H

#include "Connection.h"
#include "Packet.h"
#include "Player.h"
#include "NPC.h"
#include "Map.h"

enum {
	CHARACTER_CLOSE_DISTANCE = 50
};

class Game {
public:
	void process(Connection& connection, Packet& packet);
	void logic();
	void load();
	
	void disconnected(const Connection& connection);
	
	// For parsing
	const NPC& getReferenceNPC(int id) const;
	const Monster& getReferenceMonster(int id) const;
	
	// For Monster & NPC	
	std::vector<Monster*> getCloseMonsters(const Character* character);
	
	void removeMonster(int id);
	
private:
	Map& getMap(int map_id);
	
	std::vector<NPC>& getNPCsOnMap(int map_id);
	std::vector<Monster>& getMonstersOnMap(int map_id);
	
	void addPlayer(const Player& player);
	Player* getPlayer(const Connection& connection);
	std::vector<Player*> getPlayersOnMap(const std::vector<Player*>& except, int map_id);
	
	void handleLogin();
	void handleGetCharacters();
	void handleUnknownPacket();
	void handleSpawn();
	void handleMove();
	
	std::vector<Player> players_;
	std::vector<Map> maps_;
	std::vector<NPC> reference_npcs_;
	std::vector<Monster> reference_monsters_;
	
	Player* current_player_;
	Connection* current_connection_;
	Packet* current_packet_;
};

#endif