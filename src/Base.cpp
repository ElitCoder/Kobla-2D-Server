#include "Base.h"
#include "Log.h"
#include "DatabaseFile.h"
#include "Config.h"
#include "NetworkCommunication.h"
#include "Game.h"
#include "ClientData.h"

using namespace std;

Config Base::settings_;
NetworkCommunication Base::network_;
Game Base::game_;

shared_ptr<Database> Base::database_;

ClientData Base::client_;

Config& Base::settings() {
	return settings_;
}

NetworkCommunication& Base::network() {
	return network_;
}

Game& Base::game() {
	return game_;
}

shared_ptr<Database>& Base::database() {
	return database_;
}

ClientData& Base::client() {
	return client_;
}

void Base::createDatabase(int type) {
	switch (type) {
		case DATABASE_TYPE_FILE: database_ = make_shared<DatabaseFile>();
			break;
			
		default: Log(WARNING) << "Unknown type of Database " << type << endl;
	}
}