#include "SpawnHandler.h"
#include "Random.h"
#include "Timer.h"
#include "Monster.h"

#include <algorithm>

using namespace std;

void SpawnHandler::addToRespawn(const Monster& monster) {
	auto spawn_time = Random::getRandomInteger(SPAWN_HANDLER_MONSTER_MIN_MS, SPAWN_HANDLER_MONSTER_MAX_MS);
	
	spawn_.push_back({ monster, Timer(spawn_time) });
}

vector<Monster> SpawnHandler::getRespawn() {
	vector<Monster> respawn;
	
	for (const auto& peer : spawn_) {
		auto& monster = peer.first;
		auto& timer = peer.second;
		
		// Only respawn the Monsters that are waiting to be respawned
		if (!timer.elapsed())
			continue;
			
		respawn.push_back(monster);	
	}
	
	// Remove the respawned monsters
	if (!respawn.empty()) {
		spawn_.erase(remove_if(spawn_.begin(), spawn_.end(), [] (auto& peer) {
			auto& timer = peer.second;
			
			return timer.elapsed();
		}));
	}
	
	return respawn;
}