#include "Map.h"

using namespace std;

int Map::getID() const {
	return id_;
}

vector<NPC>& Map::getNPCs() {
	return npcs_;
}

void Map::addNPC(const NPC& npc) {
	npcs_.push_back(npc);
}

void Map::setID(int id) {
	id_ = id;
}