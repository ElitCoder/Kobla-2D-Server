#pragma once
#ifndef DATABASE_FILE_H
#define DATABASE_FILE_H

#include "Database.h"

class DatabaseFile : public Database {
public:
	virtual bool login(const std::string& username, const std::string& password) override;
	
	virtual void parseNPCs(std::vector<NPC>& reference_npcs) override;
	virtual void parseMaps(std::vector<Map>& maps) override;
	virtual void parseMonsters(std::vector<Monster>& reference_monsters) override;
	virtual void parseActions(std::vector<Action>& reference_actions) override;
};

#endif