#include "Monster.h"
#include "Base.h"
#include "Log.h"

using namespace std;

void Monster::setMonsterID(int id) {
	id_ = id;
}

int Monster::getMonsterID() const {
	return id_;
}