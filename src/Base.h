#pragma once
#ifndef BASE_H
#define BASE_H

#include <memory>

class NetworkCommunication;
class Game;
class Database;
class ClientData;
class Config;

class Base {
public:
	static Config& settings();
	static NetworkCommunication& network();
	static Game& game();
	static std::shared_ptr<Database>& database();
	static ClientData& client();
	
	static void createDatabase(int type);
	
private:
	static Config settings_;
	static NetworkCommunication network_;
	static Game game_;
	
	// Database is an abstract class to enable different types
	static std::shared_ptr<Database> database_;
	
	static ClientData client_;
};

#endif