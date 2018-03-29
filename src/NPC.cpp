#include "NPC.h"

void NPC::setNPCID(size_t id) {
	npc_id_ = id;
}

size_t NPC::getNPCID() const {
	return npc_id_;
}