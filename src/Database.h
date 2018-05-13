#pragma once
#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

enum {
	DATABASE_TYPE_FILE
};

class Map;
class NPC;
class Monster;
class Action;

class Database {
public:
	virtual ~Database();
	
	// Loading
	virtual void parseNPCs(std::vector<NPC>& reference_npcs) = 0;
	virtual void parseMaps(std::vector<Map>& maps) = 0;
	virtual void parseMonsters(std::vector<Monster>& monsters) = 0;
	virtual void parseActions(std::vector<Action>& reference_actions) = 0;
		
	// Operations
	virtual bool login(const std::string& username, const std::string& password) = 0;
	
protected:
	Database();
};

#endif