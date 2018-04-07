#ifndef MAP_H
#define MAP_H

#include "NPC.h"
#include "Monster.h"

#include <vector>
#include <cstddef>
#include <array>

class MapSpawnPoint {
public:
	MapSpawnPoint(const std::array<int, 2>& from, const std::array<int, 2>& to);
	
	int getFromX() const;
	int getFromY() const;
	int getToX() const;
	int getToY() const;
	
private:
	std::array<int, 2> from_;
	std::array<int, 2> to_;
};

class Map {
public:
	void setID(int id);
	void addNPC(const NPC& npc);
	void addMonster(const Monster& monster, int number = 1, const MapSpawnPoint& point = MapSpawnPoint({ 0, 0 }, { 0, 0 }));
	void addSpawnPoint(const MapSpawnPoint& point);
	void removeMonster(int id);
	
	int getID() const;
	std::vector<NPC>& getNPCs();
	std::vector<Monster>& getMonsters();
	std::array<int, 2> getSpawnPoint();
	
	void react();
		
private:
	int id_;
	std::vector<NPC> npcs_;
	std::vector<Monster> monsters_;
	std::vector<MapSpawnPoint> player_spawn_points_;
};

#endif