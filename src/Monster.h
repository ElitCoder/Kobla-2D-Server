#ifndef MONSTER_H
#define MONSTER_H

#include "Character.h"
#include "AI.h"

class Monster : public Character {
public:
	Monster();
	~Monster();
	
	Monster(const Monster& monster);
	Monster(Monster&& monster) = delete;
	
	Monster& operator=(const Monster& monster) = delete;
	Monster& operator=(Monster&& monster) = delete;
	
	void setMonsterID(int id);
	int getMonsterID() const;
	
	void bindAI(int type);
	void react();

private:
	int monster_id_;
	AI* ai_;
};

#endif