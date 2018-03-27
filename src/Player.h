#ifndef PLAYER_H
#define PLAYER_H

#include "Character.h"

class Player : public Character {
public:
	Player();
	
	int getConnectionID() const;
	
private:
	// Use Connection getSocket() for ID
	int connection_id_;
};

#endif