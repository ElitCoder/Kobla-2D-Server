#ifndef PLAYER_H
#define PLAYER_H

#include "Character.h"

class Player : public Character {
public:
	Player();
	
	size_t getConnectionID() const;
	void setConnectionID(size_t id);
	
private:
	// Use Connection getUniqueID() for ID
	size_t connection_id_;
};

#endif