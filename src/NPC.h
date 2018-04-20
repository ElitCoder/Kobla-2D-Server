#ifndef NPC_H
#define NPC_H

#include "AI.h"

class NPC : public AI {
public:
	void setNPCID(size_t id);
	size_t getNPCID() const;
	
private:
	size_t npc_id_;
};

#endif