#include "AI.h"
#include "Base.h"
#include "PacketCreator.h"

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
		// This AI is Monster-exclusive, we know it's a Monster
		Monster* me = (Monster*)this;
		
		// Following trumps everything else
		if (me->isFollowing()) {
			// Get distance to player and calculate curve to player's location
			
		} else {
			// Are any players close? Let's follow them in that case
			auto players = Base::game().getClosePlayers(this);
		}
		
		// Let's update direction
		if (!me->isMoving()) {
			// Have we waited long enough?
			if (!me->strollingWaitingElapsed())
				return;
				
			me->changeMoveStatus(true, me->getX(), me->getY(), (me->getMovingDirection() + 1) % PLAYER_MOVE_MAX);
			Base::game().updateMovement(me, {});
		} else {
			if (me->getDistanceMoved() >= CHARACTER_CASUAL_STROLLING_DISTANCE) {
				// Stop movement
				me->startStrollingWaiting();
				
				me->changeMoveStatus(false, me->getX(), me->getY(), me->getMovingDirection());
				Base::game().updateMovement(me, {});
			}
		}
	}
}