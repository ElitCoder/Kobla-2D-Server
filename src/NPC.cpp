#include "NPC.h"
#include "Base.h"
#include "Log.h"

using namespace std;

void NPC::setNPCID(size_t id) {
	npc_id_ = id;
}

size_t NPC::getNPCID() const {
	return npc_id_;
}