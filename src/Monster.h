#ifndef MONSTER_H
#define MONSTER_H

#include "Character.h"
#include "AI.h"

class Monster : public AI {
public:
	void setMonsterID(int id);
	int getMonsterID() const;
	
private:
	int monster_id_;
};

#endif