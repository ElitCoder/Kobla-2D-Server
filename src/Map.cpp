#include "Map.h"
#include "Random.h"
#include "Log.h"
#include "AI.h"

#include <algorithm>

using namespace std;

int Map::getID() const {
	return id_;
}

vector<NPC>& Map::getNPCs() {
	return npcs_;
}

void Map::addNPC(const NPC& npc) {
	npcs_.push_back(npc);
}

void Map::setID(int id) {
	id_ = id;
}

void Map::addSpawnPoint(const MapSpawnPoint& point) {
	player_spawn_points_.push_back(point);
}

array<int, 2> Map::getSpawnPoint() {
	int which_point = Random::getRandomInteger(0, player_spawn_points_.size() - 1);
	auto& point = player_spawn_points_.at(which_point);
	
	int x = Random::getRandomInteger(point.getFromX(), point.getToX());
	int y = Random::getRandomInteger(point.getFromY(), point.getToY());
	
	return { x, y };
}

vector<Monster>& Map::getMonsters() {
	return monsters_;
}

void Map::addMonster(const Monster& monster, int number, const MapSpawnPoint& point) {
	bool from_file = number > 1;
	
	for (int i = number; i > 0; i--) {
		Monster new_monster = monster;
		new_monster.setValidID();
		
		if (from_file) {
			int x = Random::getRandomInteger(point.getFromX(), point.getToX());
			int y = Random::getRandomInteger(point.getFromY(), point.getToY());
			
			new_monster.setPosition(x, y);
			
			Log(DEBUG) << "Adding monster " << new_monster.getID() << " at " << x << " " << y << endl;
		}
		
		// Set AI for monster
		new_monster.bindAI(AI_TYPE_BASIC);
		
		monsters_.push_back(new_monster);
	}
}

// Do logic stuff on map
void Map::react() {
	// Do Monster AI
	for (auto& monster : monsters_)
		monster.react();
		
	// Do NPC AI
	for (auto& npc : npcs_)
		npc.react();
}

void Map::removeMonster(int id) {
	// TODO: FIX THIS PRONTO
	
	/*
	
	// Remove in server
	for (size_t i = 0; i < monsters_.size(); i++) {
		if (monsters_.at(i).getID() == (unsigned int)id) {
			monsters_.erase(monsters_.begin() + i);
			
			break;
		}
	}
	*/
	
	/*
	monsters_.erase(remove_if(monsters_.begin(), monsters_.end(), [&id] (auto& monster) {
		return monster.getID() == (unsigned int)id;
	}));
	*/
}

/*
	MapSpawnPoint
*/

MapSpawnPoint::MapSpawnPoint(const array<int, 2>& from, const array<int, 2>& to) {
	from_ = from;
	to_ = to;
}

int MapSpawnPoint::getFromX() const {
	return from_.front();
}

int MapSpawnPoint::getFromY() const {
	return from_.back();
}

int MapSpawnPoint::getToX() const {
	return to_.front();
}

int MapSpawnPoint::getToY() const {
	return to_.back();
}