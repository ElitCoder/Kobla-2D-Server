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
	
private:
	size_t npc_id_ = 0;
	
	std::vector<std::pair<double, double>> patrolling_area_;
	int patrol_wait_min_;
	int patrol_wait_max_;
};

#endif