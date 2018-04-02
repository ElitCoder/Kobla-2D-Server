#include "NPC.h"
#include "Base.h"
#include "Log.h"

using namespace std;

NPC::NPC() {
	ai_ = nullptr;
}

NPC::~NPC() {
	if (ai_ != nullptr)
		delete ai_;
}

NPC::NPC(const NPC& npc) : Character(npc) {
	npc_id_ = npc.npc_id_;
	ai_ = nullptr;
	
	if (npc.ai_ == nullptr)
		return;
		
	int ai_type = npc.ai_->getType();
	ai_ = Base::game().getAI(ai_type);
}

void NPC::setNPCID(size_t id) {
	npc_id_ = id;
}

size_t NPC::getNPCID() const {
	return npc_id_;
}

void NPC::bindAI(int type) {
	Log(DEBUG) << "Binding NPC " << npc_id_ << " to AI " << type << endl;
	
	ai_ = Base::game().getAI(type);
}

void NPC::react() {
	if (ai_ == nullptr)
		return;
		
	ai_->decide(*this);
}