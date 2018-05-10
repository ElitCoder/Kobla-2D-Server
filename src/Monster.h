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
	
	bool isFollowing() const;
	
	void startStrollingWaiting();
	bool strollingWaitingElapsed();
	
	void setSpawnPoint(const MapSpawnPoint& point);
	const MapSpawnPoint& getSpawnPoint() const;
	
private:
	int monster_id_			= -1;
	Timer next_strolling_;
	bool following_			= false;
	MapSpawnPoint spawn_point_;
};

#endif