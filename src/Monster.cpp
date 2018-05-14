#include "Monster.h"
#include "Base.h"
#include "Random.h"

using namespace std;

Monster::Monster() {
	// Monster should have lower movement speed
	moving_speed_ /= 2;
	
	object_type_ = OBJECT_TYPE_MONSTER;
}

void Monster::setMonsterID(int id) {
	monster_id_ = id;
}

int Monster::getMonsterID() const {
	return monster_id_;
}

void Monster::setSpawnPoint(const MapSpawnPoint &point) {
	spawn_point_ = point;
}

const MapSpawnPoint& Monster::getSpawnPoint() const {
	return spawn_point_;
}