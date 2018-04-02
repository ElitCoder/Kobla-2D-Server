#ifndef NPC_H
#define NPC_H

#include "Character.h"
#include "AI.h"

class NPC : public Character {
public:
	NPC();
	~NPC();
	
	NPC(const NPC& npc);
	NPC(NPC&& npc) = delete;
	
	NPC& operator=(const NPC& npc) = delete;
	NPC& operator=(NPC&& npc) = delete;
	
	void setNPCID(size_t id);
	size_t getNPCID() const;
	
	void bindAI(int type);
	void react();
	
private:
	size_t npc_id_;
	AI* ai_;
};

#endif