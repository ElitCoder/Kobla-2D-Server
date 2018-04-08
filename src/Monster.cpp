#include "Monster.h"
#include "Base.h"
#include "Random.h"

using namespace std;

void Monster::setMonsterID(int id) {
	monster_id_ = id;
}

int Monster::getMonsterID() const {
	return monster_id_;
}

void Monster::startStrollingWaiting() {
	next_strolling_.start(Random::getRandomInteger(CHARACTER_CASUAL_STROLLING_WAITING_MIN_MS, CHARACTER_CASUAL_STROLLING_WAITING_MAX_MS));
}

bool Monster::strollingWaitingElapsed() {
	return next_strolling_.elapsed();
}

bool Monster::isFollowing() const {
	return following_;
}