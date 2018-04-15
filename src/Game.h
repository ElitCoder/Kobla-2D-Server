#ifndef GAME_H
#define GAME_H

#include "Connection.h"
#include "Packet.h"
#include "Player.h"
#include "NPC.h"
#include "Map.h"

enum {
	CHARACTER_CLOSE_DISTANCE = 50,
	CHARACTER_CASUAL_STROLLING_DISTANCE = 150,
	CHARACTER_CASUAL_STROLLING_WAITING_MIN_MS = 100,
	CHARACTER_CASUAL_STROLLING_WAITING_MAX_MS = 2000,
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
	std::vector<Player*> getClosePlayers(const Character* character);
	
	void removeMonster(int id);
	
	// For easy access in AI
	void updateMovement(Character* character, const std::vector<int>& sockets);
	Map& getMap(int map_id);
	
private:
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