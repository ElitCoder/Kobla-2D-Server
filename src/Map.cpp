#include "Map.h"
#include "Random.h"
#include "Log.h"
#include "AI.h"
#include "Base.h"

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
	
	return {{ x, y }};
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
			
			//Log(DEBUG) << "Adding monster " << new_monster.getID() << " at " << x << " " << y << endl;
		}
		
		// Set AI for monster
		new_monster.setAI(AI_MONSTER_TYPE_NORMAL);
		
		monsters_.push_back(new_monster);
	}
}

// Do logic stuff on map
void Map::react() {
	// Do Monster AI & movement
	for (auto& monster : monsters_) {
		monster.react();
		monster.move();
	}
		
	// Do NPC AI & movement
	for (auto& npc : npcs_) {
		npc.react();
		npc.move();
	}
}

void Map::removeMonster(int id) {
	monsters_.erase(remove_if(monsters_.begin(), monsters_.end(), [&id] (auto& monster) {
		return monster.getID() == (unsigned int)id;
	}));
}

// Is it possible to move this distance in any direction without causing collision?
int Map::getPossibleMove(const Character* character, double distance, int desired_direction) {
	int possible_direction = desired_direction;
	
	do {
		// See distance to collision in desired_direction direction
		if (Base::client().isMovePossible(character, distance, possible_direction))
			return possible_direction;
			
		possible_direction++;
		possible_direction %= PLAYER_MOVE_MAX;	
	} while (possible_direction != desired_direction);
	
	return -1;
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