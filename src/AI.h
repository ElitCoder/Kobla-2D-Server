#ifndef AI_H
#define AI_H

class NPC;
class Monster;

enum {
	AI_TYPE_BASIC
};

// AI is an interface for different types of AI
class AI {
public:
	virtual ~AI();
	
	virtual int getType() = 0;
	virtual void decide(Monster& monster) = 0;
	virtual void decide(NPC& npc) = 0;
	
protected:
	AI();
	
	int type_;
};

#endif