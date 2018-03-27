#include "Player.h"
#include "Log.h"

Player::Player() {
	connection_id_ = -1;
	
	Log(DEBUG) << "Running constructor for Player\n";
}

int Player::getConnectionID() const {
	return connection_id_;
}