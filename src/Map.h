#ifndef MAP_H
#define MAP_H

#include "NPC.h"

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
	void addSpawnPoint(const MapSpawnPoint& point);
	
	int getID() const;
	std::vector<NPC>& getNPCs();
	std::array<int, 2> getSpawnPoint();
	
private:
	int id_;
	std::vector<NPC> npcs_;
	std::vector<MapSpawnPoint> player_spawn_points_;
};

#endif