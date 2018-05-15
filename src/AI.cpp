#include "AI.h"
#include "Random.h"
#include "Base.h"
#include "Game.h"
#include "Monster.h"
#include "Map.h"

using namespace std;

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
	// Let's update direction
	if (!me->isMoving()) {
		// Have we waited long enough?
		if (!me->AIWaitingElapsed())
			return;
		
		auto desired_direction = Random::getRandomInteger(PLAYER_MOVE_RIGHT, PLAYER_MOVE_UP);
		auto distance_to_move = Random::getRandomInteger(1, CHARACTER_CASUAL_STROLLING_DISTANCE);
		auto possible_move = Base::game().getMap(me->getMapID()).getPossibleMove(me, distance_to_move, desired_direction);
				
		if (possible_move < 0)
			// No moves available
			return;

		me->changeMoveStatus(true, me->getX(), me->getY(), possible_move);
		me->setDeterminedDestination(possible_move, distance_to_move);
		
		Base::game().updateMovement(me, {});
	} else {
		if (me->reachedDeterminedDistance()) {
			// Set to destination position
			me->changeMoveStatus(false, me->getDeterminedDestination().first, me->getDeterminedDestination().second, me->getMovingDirection());
			
			// Stop movement
			me->startAIWaiting();
			
			// No need to send stop packet, the Client takes care of it
			//Base::game().updateMovement(me, {});
		}
	}
}

static void patrol(NPC* npc) {
	if (!npc->isMoving()) {
		if (!npc->AIWaitingElapsed())
			return;
			
		npc->changeMoveStatus(true, npc->getX(), npc->getY(), npc->getMovingDirection());
		npc->setDeterminedDestination(npc->setNextPatrolPosition());
		
		Base::game().updateMovement(npc, {});
	} else {
		if (npc->reachedDeterminedDistance()) {
			npc->changeMoveStatus(false, npc->getDeterminedDestination().first, npc->getDeterminedDestination().second, npc->getMovingDirection());
			
			npc->startAIWaiting();
		}
	}
}

static void hurtCharacters(int type, Character* character) {
	if (!character->canAttack())
		return;

	vector<Character*> close;
	
	if (type == OBJECT_TYPE_PLAYER) {
		// Hurt Players in contact
		auto players = Base::game().getContactPlayers(character);
		for_each(players.begin(), players.end(), [&close] (auto* player) { close.push_back(player); });
	} else {
		// Remove monsters on sight
		auto monsters = Base::game().getCloseMonsters(character);
		for_each(monsters.begin(), monsters.end(), [&close] (auto* monster) { close.push_back(monster); });
	}
	
	if (close.empty())
		return;
		
	// Hurt the type
	for (auto* player : close)
		character->attack(player);
}

void AI::initializeAI() {
	switch (ai_type_) {
		case AI_MONSTER_TYPE_NORMAL: {			
			// Set waiting type to strolling waiting
			waiting_min_ = CHARACTER_CASUAL_STROLLING_WAITING_MIN_MS;
			waiting_max_ = CHARACTER_CASUAL_STROLLING_WAITING_MAX_MS;
			
			break;
		}
		
		case AI_NPC_TYPE_PATROL: {
			// We know it's a NPC
			NPC* npc = (NPC*)this;
			
			waiting_min_ = npc->getPatrolWaitMinTime();
			waiting_max_ = npc->getPatrolWaitMaxTime();
			
			break;
		}
	}
	
	startAIWaiting();
}

void AI::react() {
	// NPC AI
	if (ai_type_ == AI_NPC_TYPE_KILL_CLOSE) {
		hurtCharacters(OBJECT_TYPE_MONSTER, this);
	} else if (ai_type_ == AI_NPC_TYPE_PATROL) {
		NPC* npc = (NPC*)this;
		
		patrol(npc);
		hurtCharacters(OBJECT_TYPE_MONSTER, this);
	}
	
	// Monster AI
	if (ai_type_ == AI_MONSTER_TYPE_NORMAL) {
		// This AI is Monster-exclusive, we know it's a Monster
		Monster* me = (Monster*)this;
		
		strollingMove(me);
		hurtCharacters(OBJECT_TYPE_PLAYER, me);
	}
}