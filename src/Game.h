#pragma once
#ifndef GAME_H
#define GAME_H

#include "Monster.h"
#include "Player.h"
#include "NPC.h"
#include "Map.h"
#include "Action.h"

#include <SFML/Graphics/Rect.hpp>

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
	const Action& getReferenceAction(int id) const;
	
	// For Monster & NPC	
	std::vector<Monster*> getCloseMonsters(const Character* character);
	std::vector<Player*> getClosePlayers(const Character* character);
	std::vector<Player*> getContactPlayers(const Character* character);
	
	Object* getObject(int id);
	
	void removeCharacter(int id);
	void removeObject(const Object* object);
	
	void spawnCharacter(const Character* character);
	
	// For easy access in AI
	void updateMovement(const Object* object, const std::vector<int>& sockets);
	Map& getMap(int map_id);
	
	// For collision detection
	bool isCollision(const sf::FloatRect& box, Object* object);
	
private:
	std::vector<NPC>& getNPCsOnMap(int map_id);
	std::vector<Monster>& getMonstersOnMap(int map_id);
	std::vector<TemporaryObject>& getObjectsOnMap(int map_id);
	
	void addPlayer(const Player& player);
	Player* getPlayer(size_t connection_id);
	std::vector<Player*> getPlayersOnMap(const std::vector<int>& except_ids, int map_id);
	
	void handleLogin();
	void handleGetCharacters();
	void handleUnknownPacket();
	void handleSpawn();
	void handleMove();
	void handleShoot();
	void handleHit();
	void handleActivate();
	void handleChat();
	
	void disconnect();
	
	std::vector<Player> players_;
	std::vector<Map> maps_;
	std::vector<NPC> reference_npcs_;
	std::vector<Monster> reference_monsters_;
	std::vector<Action> reference_actions_;
	
	Player* current_player_;
	Connection* current_connection_;
	Packet* current_packet_;
	
	Timer last_forced_logic_;
	
	// Keep track of any Players to disconnect <socket_id, connection_id>
	std::deque<std::pair<int, size_t>> disconnects_;
	std::mutex disconnects_mutex_;
};

#endif