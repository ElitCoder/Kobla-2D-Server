#pragma once
#ifndef MONSTER_H
#define MONSTER_H

#include "AI.h"
#include "Map.h"

class Monster : public AI {
public:
	Monster();
	
	void setMonsterID(int id);
	int getMonsterID() const;
	
	void setSpawnPoint(const MapSpawnPoint& point);
	const MapSpawnPoint& getSpawnPoint() const;
	
private:
	int monster_id_ = -1;

	MapSpawnPoint spawn_point_;
};

#endif