#ifndef BASE_H
#define BASE_H

#include "Config.h"
#include "NetworkCommunication.h"
#include "Game.h"
#include "Database.h"

class Base {
public:
	static Config& settings();
	static NetworkCommunication& network();
	static Game& game();
	static Database* database();
	
	static void createDatabase(int type);
	static void destroyDatabase();
	
private:
	static Config settings_;
	static NetworkCommunication network_;
	static Game game_;
	
	// Database is an abstract class to enable different types
	static Database* database_;
};

#endif