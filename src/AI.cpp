#include "AI.h"
#include "Random.h"
#include "Base.h"
#include "Game.h"
#include "Monster.h"
#include "Map.h"

AI::AI() {
	ai_type_ = AI_TYPE_NONE;
}

void AI::setAI(int type) {
	ai_type_ = type;
}

static void strollingMove(Monster* me) {
	// Following trumps everything else
	if (me->isFollowing()) {
		// Get distance to player and calculate curve to player's location
	} else {
		// Are any players close? Let's follow them in that case
		auto players = Base::game().getClosePlayers(me);
	}
	
	// Let's update direction
	if (!me->isMoving()) {
		// Have we waited long enough?
		if (!me->strollingWaitingElapsed())
			return;
		
		//auto desired_direction = (me->getMovingDirection() + 1) % PLAYER_MOVE_MAX;
		auto desired_direction = Random::getRandomInteger(PLAYER_MOVE_RIGHT, PLAYER_MOVE_UP);
		auto distance_to_move = Random::getRandomInteger(CHARACTER_CASUAL_STROLLING_DISTANCE / 10, CHARACTER_CASUAL_STROLLING_DISTANCE);
		auto possible_move = Base::game().getMap(me->getMapID()).getPossibleMove(me, distance_to_move, desired_direction);
				
		if (possible_move < 0)
			// No moves available
			return;

		me->changeMoveStatus(true, me->getX(), me->getY(), possible_move);
		me->setPredeterminedDistance(distance_to_move);
		
		Base::game().updateMovement(me, {});
	} else {
		if (me->getDistanceMoved() >= me->getPredeterminedDistance()) {
			// Stop movement
			me->startStrollingWaiting();
			
			me->changeMoveStatus(false, me->getX(), me->getY(), me->getMovingDirection());
			
			// No need to send stop packet, the Client takes care of it
			//Base::game().updateMovement(me, {});
		}
	}
}

void AI::initializeAI() {
	switch (ai_type_) {
		case AI_MONSTER_TYPE_NORMAL: {
			// Set random movement direction so not every Monster starts with moving the same direction
			auto direction = Random::getRandomInteger(PLAYER_MOVE_RIGHT, PLAYER_MOVE_UP);
			
			setMovingDirection(direction);
			
			// We know that this is a Monster AI
			Monster* monster = (Monster*)this;
			monster->startStrollingWaiting();
			
			break;
		}
	}
}

void AI::react() {
	if (ai_type_ == AI_NPC_TYPE_KILL_CLOSE) {
		// Remove monsters on sight
		auto monsters = Base::game().getCloseMonsters(this);
		
		for (auto& monster : monsters)
			Base::game().removeMonster(monster->getID());
	} else if (ai_type_ == AI_MONSTER_TYPE_NORMAL) {
		// This AI is Monster-exclusive, we know it's a Monster
		Monster* me = (Monster*)this;
		
		strollingMove(me);
	}
}