#include "NPC.h"
#include "Base.h"
#include "Log.h"

#include <cmath>

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

int NPC::getPatrolWaitMinTime() const {
	return patrol_wait_min_;
}

int NPC::getPatrolWaitMaxTime() const {
	return patrol_wait_max_;
}

void NPC::setNextPatrolPosition() {
	current_patrol_position_++;
	current_patrol_position_ %= patrolling_area_.size();
}

bool NPC::reachedPatrolPosition() const {
	auto patrol_position = patrolling_area_.at(current_patrol_position_);
	
	// Get distance to position
	double distance = pow(patrol_position.first - getX(), 2) + pow(patrol_position.second - getY(), 2);
	
	return sqrt(distance) < 5;
}

int NPC::getPatrolDirection() const {
	auto patrol_position = patrolling_area_.at(current_patrol_position_);

	double x_distance = patrol_position.first - getX();
	double y_distance = patrol_position.second - getY();
	
	if (abs(x_distance) > abs(y_distance)) {
		if (x_distance < 0)
			return PLAYER_MOVE_LEFT;
		else
			return PLAYER_MOVE_RIGHT;
	} else {
		if (y_distance < 0)
			return PLAYER_MOVE_UP;
		else
			return PLAYER_MOVE_DOWN;
	}
	
	return -1;
}