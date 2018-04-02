#ifndef MAP_H
#define MAP_H

#include "NPC.h"

#include <vector>
#include <cstddef>

class Map {
public:
	void setID(int id);
	void addNPC(const NPC& npc);
	
	int getID() const;
	std::vector<NPC>& getNPCs();
	
private:
	int id_;
	std::vector<NPC> npcs_;
};

#endif