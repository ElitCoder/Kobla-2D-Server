#include "Player.h"

Player::Player() {
	static size_t id;
	
	id_ = id++;
	connection_id_ = -1;
}

int Player::getConnectionID() const {
	return connection_id_;
}