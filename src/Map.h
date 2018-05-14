#pragma once
#ifndef MAP_H
#define MAP_H

#include "TemporaryObject.h"
#include "SpawnHandler.h"

#include <vector>
#include <cstddef>
#include <array>

class NPC;
class Monster;
class Character;

class MapSpawnPoint {
public:
	MapSpawnPoint();
	MapSpawnPoint(const std::array<int, 2>& from, const std::array<int, 2>& to);
	
	int getFromX() const;
	int getFromY() const;
	int getToX() const;
	int getToY() const;
	
	void setLimit(bool limit);
	bool hasLimit() const;
	
private:
	std::array<int, 2> from_;
	std::array<int, 2> to_;
	
	bool limit_ = false;
};

class Map {
public:
	void setID(int id);
	void addNPC(const NPC& npc);
	void addMonster(const Monster& monster, int number = 1, const MapSpawnPoint& point = MapSpawnPoint({{ 0, 0 }}, {{ 0, 0 }}));
	void addSpawnPoint(const MapSpawnPoint& point);
	void removeMonster(int id);
	void removeObjects(const std::vector<int>& ids);
	void addObject(const TemporaryObject& object);
	
	int getID() const;
	std::vector<NPC>& getNPCs();
	std::vector<Monster>& getMonsters();
	std::array<int, 2> getSpawnPoint();
	std::vector<TemporaryObject>& getTemporaryObjects();
	Object* getObject(int id);
	
	int getPossibleMove(const Character* character, double distance, int desired_direction);
	void checkHit(const Character* shooter, int bullet_id, int hit_id);
	
	void react();
		
private:
	void objectHit(Object* object);
	
	TemporaryObject* getTemporaryObject(int id);
	Character* getCharacter(int id);
	
	int id_;
	
	std::vector<NPC> npcs_;
	std::vector<Monster> monsters_;
	std::vector<MapSpawnPoint> player_spawn_points_;
	std::vector<TemporaryObject> objects_;
	
	SpawnHandler spawn_handler_;
};

#endif