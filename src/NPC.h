#pragma once
#ifndef NPC_H
#define NPC_H

#include "AI.h"

class NPC : public AI {
public:
	NPC();
	
	void setNPCID(size_t id);
	size_t getNPCID() const;
	
	void setPatrol(const std::vector<std::pair<double, double>>& patrol);
	void setPatrolWaitTime(int min, int max);
	
	int getPatrolWaitMinTime() const;
	int getPatrolWaitMaxTime() const;
	
	void setNextPatrolPosition();
	
	bool reachedPatrolPosition() const;
	int getPatrolDirection() const;
	
private:
	size_t npc_id_ = 0;
	
	std::vector<std::pair<double, double>> patrolling_area_;
	int patrol_wait_min_ = -1;
	int patrol_wait_max_ = -1;
	
	int current_patrol_position_ = 0;
};

#endif