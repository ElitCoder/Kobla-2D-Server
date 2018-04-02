#include "AIBasic.h"
#include "Log.h"
#include "Base.h"

AIBasic::AIBasic() {
	type_ = AI_TYPE_BASIC;
}

int AIBasic::getType() {
	return type_;
}

void AIBasic::decide(Monster& monster) {
	// The monster is some kind of state, what should we do?
	// Move 2 sec, then wait for 1 sec
	
	// Just FUCKING MOVE TO THE LEFT YOLO
	//monster.changeMoveStatus(true, monster.getX(), monster.getY(), PLAYER_MOVE_LEFT);
}

void AIBasic::decide(NPC& npc) {
	// Are we near any monsters?
	auto monsters = Base::game().getCloseMonsters(&npc);
	
	for (auto monster : monsters) {
		Base::game().removeMonster(monster->getID());
	}
}