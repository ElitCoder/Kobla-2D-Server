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
	static Database& database();
	
private:
	static Config settings_;
	static NetworkCommunication network_;
	static Game game_;
	static Database database_;
};

#endif