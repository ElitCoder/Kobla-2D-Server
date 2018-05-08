#include "Character.h"
#include "Log.h"

using namespace std;

size_t g_character_id = 0;

Character::Character() {
	id_ = g_character_id++;
	
	Log(DEBUG) << "Running constructor for Character\n";
	
	moving_ = false;
	
	// Hard coded for now
	x_ = 100;
	y_ = 100;
	object_id_ = 0;
	map_id_ = 0;
	moving_speed_ = 200;
	full_health_ = 100;
	current_health_ = full_health_;
	
	name_ = "Igge" + to_string(getID());
	
	// Collide with everything except Monsters & Players
	setCollision(COLLISION_MAP, true);
	setCollision(COLLISION_MONSTERS, false);
	setCollision(COLLISION_NPCS, true);
	setCollision(COLLISION_PLAYERS, false);
}

double Character::getCurrentHealth() const {
	return current_health_;
}

double Character::getFullHealth() const {
	return full_health_;
}

void Character::setColliding(bool status) {
	colliding_ = status;
}

bool Character::isColliding() const {
	return colliding_;
}