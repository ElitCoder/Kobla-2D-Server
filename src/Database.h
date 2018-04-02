#ifndef DATABASE_H
#define DATABASE_H

#include "Map.h"

#include <string>

enum {
	DATABASE_TYPE_FILE
};

class Database {
public:
	virtual ~Database();
	
	// Loading
	virtual void parseNPCs(std::vector<NPC>& reference_npcs) = 0;
	virtual void parseMaps(std::vector<Map>& maps) = 0;
		
	// Operations
	virtual bool login(const std::string& username, const std::string& password) = 0;
	
protected:
	Database();
};

#endif