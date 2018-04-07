#include "Player.h"
#include "Log.h"

Player::Player() {
	connection_id_ = 0;
	
	// Set player movement a little higher for now
	moving_speed_ *= 2;
	
	Log(DEBUG) << "Running constructor for Player\n";
}

void Player::setConnectionID(size_t id) {
	connection_id_ = id;
}

size_t Player::getConnectionID() const {
	return connection_id_;
}