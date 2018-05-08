#include "NPC.h"
#include "Base.h"
#include "Log.h"

using namespace std;

NPC::NPC() {
	object_type_ = OBJECT_TYPE_NPC;
}

void NPC::setNPCID(size_t id) {
	npc_id_ = id;
}

size_t NPC::getNPCID() const {
	return npc_id_;
}