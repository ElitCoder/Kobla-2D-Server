#ifndef MONSTER_H
#define MONSTER_H

#include "Character.h"

class Monster : public Character {
public:
	void setMonsterID(int id);
	int getMonsterID() const;

private:
	int monster_id_;
};

#endif