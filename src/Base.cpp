#include "Base.h"
#include "Log.h"
#include "DatabaseFile.h"

using namespace std;

Config Base::settings_;
NetworkCommunication Base::network_;
Game Base::game_;

Database* Base::database_ = nullptr;

Config& Base::settings() {
	return settings_;
}

NetworkCommunication& Base::network() {
	return network_;
}

Game& Base::game() {
	return game_;
}

Database* Base::database() {
	return database_;
}

void Base::createDatabase(int type) {
	switch (type) {
		case DATABASE_TYPE_FILE: database_ = new DatabaseFile;
			break;
			
		default: Log(WARNING) << "Unknown type of Database " << type << endl;
	}
}

void Base::destroyDatabase() {
	if (database_ == nullptr)
		return;
		
	delete database_;
}