#pragma once
#ifndef AI_H
#define AI_H

#include "Character.h"

/*
	AI_TYPE_NONE				- not set
	AI_MONSTER_TYPE_AGGRESSIVE	- follows and attack on sight
	AI_MONSTER_TYPE_NORMAL		- moves around "casually"
	AI_NPC_TYPE_KILL_CLOSE		- kills monster if they're close to protect the town!
	AI_NPC_TYPE_PATROL			- extension to AI_NPC_TYPE_KILL_CLOSE with patrolling
*/
enum {
	AI_TYPE_NONE,
	AI_MONSTER_TYPE_AGGRESSIVE,
	AI_MONSTER_TYPE_NORMAL,
	AI_NPC_TYPE_KILL_CLOSE,
	AI_NPC_TYPE_PATROL
};

enum {
	CHARACTER_CASUAL_STROLLING_WAITING_MIN_MS = 100,
	CHARACTER_CASUAL_STROLLING_WAITING_MAX_MS = 2000
};

class AI : public Character {
public:
	AI();
	
	void react();
	void setAI(int type);
	void initializeAI();
	
	void startAIWaiting();
	bool AIWaitingElapsed();

private:
	int ai_type_ = -1;
	
	Timer waiting_;
	
	int waiting_min_ = -1;
	int waiting_max_ = -1;
};

#endif