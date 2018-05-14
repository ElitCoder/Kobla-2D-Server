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

void NPC::setPatrol(const vector<pair<double, double>>& patrol) {
	patrolling_area_ = patrol;
}

void NPC::setPatrolWaitTime(int min, int max) {
	patrol_wait_min_ = min;
	patrol_wait_max_ = max;
}