#ifndef AI_BASIC_H
#define AI_BASIC_H

#include "AI.h"
#include "Timer.h"

/*
	AI Basic
	
	Monster
				- Aggro if specified
				- Attacks back if attacked
				- Follows?
				
	NPC
	 			- Kills close Monsters in one shot
*/

class AIBasic : public AI {
public:
	AIBasic();
	
	virtual int getType() override;
	virtual void decide(Monster& monster) override;
	virtual void decide(NPC& npc) override;
	
private:
	Timer next_movement;
};

#endif