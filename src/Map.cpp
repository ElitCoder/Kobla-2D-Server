#include "Map.h"
#include "Random.h"
#include "Log.h"
#include "AI.h"
#include "Base.h"
#include "Monster.h"
#include "TemporaryObject.h"
#include "NPC.h"
#include "ClientData.h"
#include "Game.h"

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
	for (int i = 0; i < number; i++) {
		Monster new_monster = monster;
		new_monster.setValidID();
		
		int x = Random::getRandomInteger(point.getFromX(), point.getToX());
		int y = Random::getRandomInteger(point.getFromY(), point.getToY());
		
		new_monster.setPosition(x, y);
		
		// Set AI for monster
		new_monster.setAI(AI_MONSTER_TYPE_NORMAL);
		new_monster.initializeAI();
		
		new_monster.setSpawnPoint(point);
		
		monsters_.push_back(new_monster);
	}
}

// Do logic stuff on map
void Map::react() {
	// Do Object movement (bullets etc) and collision detection, etc
	vector<int> remove_object_ids;
	
	for (auto& object : objects_) {
		if (!object.move()) {
			// Tell the Clients that this Object is gone
			Base::game().removeObject(&object);
			
			// We hit something
			auto& information = object.getCollisionInformation();
			
			if (information.getType() == COLLISION_MONSTERS) {
				Base::game().removeMonster(information.getID());
			}
			
			remove_object_ids.push_back(object.getID());
		}
	}
	
	removeObjects(remove_object_ids);
	
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
	
	// Add respawn
	auto spawn = spawn_handler_.getRespawn();
	
	if (spawn.empty())
		return;
		
	for (auto& old_monster : spawn) {
		Monster monster = Base::game().getReferenceMonster(old_monster.getMonsterID());
		monster.setMapID(getID());
				
		addMonster(monster, 1, old_monster.getSpawnPoint());
		Base::game().spawnCharacter(&monsters_.back());
	}
}

void Map::removeObjects(const vector<int>& ids) {
	if (ids.empty())
		return;

	objects_.erase(remove_if(objects_.begin(), objects_.end(), [&ids] (auto& object) { 
		return find(ids.begin(), ids.end(), object.getID()) != ids.end();
	}), objects_.end());
}

void Map::removeMonster(int id) {
	auto iterator = find_if(monsters_.begin(), monsters_.end(), [&id] (auto& monster) { return monster.getID() == id; });
	
	if (iterator == monsters_.end()) {
		Log(WARNING) << "Trying to remove a monster that does not exist\n";
		
		return;
	}
	
	// Add to respawn?
	spawn_handler_.addToRespawn(*iterator);
	
	monsters_.erase(remove_if(monsters_.begin(), monsters_.end(), [&id] (auto& monster) {
		return monster.getID() == id;
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

void Map::addObject(const TemporaryObject& object) {
	objects_.push_back(object);
}

/*
	MapSpawnPoint
*/

MapSpawnPoint::MapSpawnPoint() {}

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