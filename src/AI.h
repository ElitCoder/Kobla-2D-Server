#ifndef AI_H
#define AI_H

#include "Character.h"

/*
	AI_TYPE_NONE				- not set
	AI_MONSTER_TYPE_AGGRESSIVE	- follows and attack on sight
	AI_MONSTER_TYPE_NORMAL		- normal, stands still for now
	AI_NPC_TYPE_KILL_CLOSE		- kills monster if they're close to protect the town!
*/
enum {
	AI_TYPE_NONE,
	AI_MONSTER_TYPE_AGGRESSIVE,
	AI_MONSTER_TYPE_NORMAL,
	AI_NPC_TYPE_KILL_CLOSE
};

class AI : public Character {
public:
	AI();
	
	void react();
	void setAI(int type);

private:
	int type_;
};

#endif