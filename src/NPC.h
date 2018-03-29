#ifndef NPC_H
#define NPC_H

#include "Character.h"

class NPC : public Character {
public:
	void setNPCID(size_t id);
	size_t getNPCID() const;
	
private:
	size_t npc_id_;
};

#endif