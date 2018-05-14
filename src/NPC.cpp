#include "NPC.h"
#include "Base.h"
#include "Log.h"

#include <cmath>

using namespace std;

NPC::NPC() {
	object_type_ = OBJECT_TYPE_NPC;
	
	// NPCs move slower
	moving_speed_ = 90;
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

int NPC::getPatrolWaitMinTime() const {
	return patrol_wait_min_;
}

int NPC::getPatrolWaitMaxTime() const {
	return patrol_wait_max_;
}

const pair<double, double>& NPC::setNextPatrolPosition() {
	current_patrol_position_++;
	current_patrol_position_ %= patrolling_area_.size();
	
	return patrolling_area_.at(current_patrol_position_);
}