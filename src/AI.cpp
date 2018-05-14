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
	
	initializeAI();
}

void AI::startAIWaiting() {
	auto waiting_ms = Random::getRandomInteger(waiting_min_, waiting_max_);
	
	waiting_.start(waiting_ms);
}

bool AI::AIWaitingElapsed() {
	return waiting_.elapsed();
}

static void strollingMove(Monster* me) {
	#if 0
	// Following trumps everything else
	if (me->isFollowing()) {
		// Get distance to player and calculate curve to player's location
	} else {
		// Are any players close? Let's follow them in that case
		auto players = Base::game().getClosePlayers(me);
	}
	#endif
	
	// Let's update direction
	if (!me->isMoving()) {
		// Have we waited long enough?
		if (!me->AIWaitingElapsed())
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
			me->startAIWaiting();
			
			me->changeMoveStatus(false, me->getX(), me->getY(), me->getMovingDirection());
			
			// No need to send stop packet, the Client takes care of it
			//Base::game().updateMovement(me, {});
		}
	}
}

static void patrol(NPC* npc) {
	if (npc->isMoving()) {
		// Are we moving?
		//if (npc->getDistanceMoved() >= npc->getPredeterminedDistance()) {
		if (npc->reachedPatrolPosition()) {
			npc->changeMoveStatus(false, npc->getX(), npc->getY(), npc->getMovingDirection());
			Base::game().updateMovement(npc, {});
			
			npc->startAIWaiting();
		} else {
			auto changed = npc->changeMoveStatus(true, npc->getX(), npc->getY(), npc->getPatrolDirection());
			
			if (changed) {
				Log(DEBUG) << "Updated movement\n";
				
				Base::game().updateMovement(npc, {});
			}
		}
	} else {
		// Should we start moving?
		if (!npc->AIWaitingElapsed())
			return;
			
		// Set target to
		npc->setNextPatrolPosition();
		auto direction = npc->getPatrolDirection();
		
		npc->changeMoveStatus(true, npc->getX(), npc->getY(), direction);
		Base::game().updateMovement(npc, {});
	}
}

void AI::initializeAI() {
	switch (ai_type_) {
		case AI_MONSTER_TYPE_NORMAL: {			
			// Set waiting type to strolling waiting
			waiting_min_ = CHARACTER_CASUAL_STROLLING_WAITING_MIN_MS;
			waiting_max_ = CHARACTER_CASUAL_STROLLING_WAITING_MAX_MS;
			
			startAIWaiting();
			
			break;
		}
		
		case AI_NPC_TYPE_PATROL: {
			// We know it's a NPC
			NPC* npc = (NPC*)this;
			
			waiting_min_ = npc->getPatrolWaitMinTime();
			waiting_max_ = npc->getPatrolWaitMaxTime();
			
			startAIWaiting();
			
			break;
		}
	}
}

void AI::react() {
	// NPC AI
	if (ai_type_ == AI_NPC_TYPE_KILL_CLOSE) {
		// Remove monsters on sight
		auto monsters = Base::game().getCloseMonsters(this);
		
		for (auto& monster : monsters)
			Base::game().removeMonster(monster->getID());
	} else if (ai_type_ == AI_NPC_TYPE_PATROL) {
		// Change patrol direction?
		NPC* npc = (NPC*)this;
		patrol(npc);
		
		// Change AI type temporarily to run AI_NPC_TYPE_KILL_CLOSE
		ai_type_ = AI_NPC_TYPE_KILL_CLOSE;
		react();
		ai_type_ = AI_NPC_TYPE_PATROL;
	}
	
	// Monster AI
	if (ai_type_ == AI_MONSTER_TYPE_NORMAL) {
		// This AI is Monster-exclusive, we know it's a Monster
		Monster* me = (Monster*)this;
		
		strollingMove(me);
	}
}