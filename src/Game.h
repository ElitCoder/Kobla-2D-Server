#ifndef GAME_H
#define GAME_H

#include "ClientData.h"
#include "Monster.h"
#include "Player.h"
#include "NPC.h"
#include "Map.h"

#include <vector>

class Connection;
class Packet;
class Character;
class Object;

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
	
	// For collision detection
	bool isCollision(const sf::FloatRect& box, const Object* object);
	
private:
	std::vector<NPC>& getNPCsOnMap(int map_id);
	std::vector<Monster>& getMonstersOnMap(int map_id);
	
	void addPlayer(const Player& player);
	Player* getPlayer(const Connection& connection);
	std::vector<Player*> getPlayersOnMap(const std::vector<int>& except_ids, int map_id);
	
	void handleLogin();
	void handleGetCharacters();
	void handleUnknownPacket();
	void handleSpawn();
	void handleMove();
	void handleShoot();
	
	std::vector<Player> players_;
	std::vector<Map> maps_;
	std::vector<NPC> reference_npcs_;
	std::vector<Monster> reference_monsters_;
	
	Player* current_player_;
	Connection* current_connection_;
	Packet* current_packet_;
};

#endif