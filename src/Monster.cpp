#include "Monster.h"
#include "Base.h"
#include "Log.h"

using namespace std;

Monster::Monster() {
	ai_ = nullptr;
}

Monster::~Monster() {
	if (ai_ != nullptr)
		delete ai_;
}

Monster::Monster(const Monster& monster) : Character(monster) {
	monster_id_ = monster.monster_id_;
	ai_ = nullptr;
	
	if (monster.ai_ == nullptr)
		return;
		
	int ai_type = monster.ai_->getType();
	ai_ = Base::game().getAI(ai_type);
}

void Monster::setMonsterID(int id) {
	id_ = id;
}

int Monster::getMonsterID() const {
	return id_;
}

void Monster::bindAI(int type) {
	Log(DEBUG) << "Binding Monster " << monster_id_ << " to AI " << type << endl;

	ai_ = Base::game().getAI(type);
}

// Do AI
void Monster::react() {
	if (ai_ == nullptr)
		return;

	ai_->decide(*this);
}