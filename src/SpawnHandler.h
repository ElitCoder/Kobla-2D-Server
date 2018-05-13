#pragma once
#ifndef SPAWN_HANDLER_H
#define SPAWN_HANDLER_H

#include <vector>

enum {
	SPAWN_HANDLER_MONSTER_MIN_MS = 30000,
	SPAWN_HANDLER_MONSTER_MAX_MS = 60000
};

class Monster;
class Timer;

class SpawnHandler {
public:
	void addToRespawn(const Monster& monster);
	std::vector<Monster> getRespawn();
	
private:
	std::vector<std::pair<Monster, Timer>> spawn_;
};

#endif