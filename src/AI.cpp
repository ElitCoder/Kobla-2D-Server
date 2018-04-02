#include "AI.h"
#include "Base.h"

AI::AI() {
	type_ = AI_TYPE_NONE;
}

void AI::setAI(int type) {
	type_ = type;
}

void AI::react() {
	if (type_ == AI_NPC_TYPE_KILL_CLOSE) {
		// Remove monsters on sight
		
		auto monsters = Base::game().getCloseMonsters(this);
		
		for (auto& monster : monsters)
			Base::game().removeMonster(monster->getID());
	} else if (type_ == AI_MONSTER_TYPE_NORMAL) {
	}
}