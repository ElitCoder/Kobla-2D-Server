#pragma once
#ifndef MONSTER_H
#define MONSTER_H

#include "AI.h"

class Monster : public AI {
public:
	Monster();
	
	void setMonsterID(int id);
	int getMonsterID() const;
	
	bool isFollowing() const;
	
	void startStrollingWaiting();
	bool strollingWaitingElapsed();
	
private:
	int monster_id_;
	Timer next_strolling_;
	bool following_			= false;
};

#endif