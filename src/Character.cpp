#include "Character.h"
#include "Log.h"

using namespace std;

Character::Character() {
	static size_t id;
	id_ = id++;
	
	Log(DEBUG) << "Running constructor for Character\n";
	
	moving_ = false;
	
	// Hard coded for now
	x_ = 100;
	y_ = 100;
	texture_id_ = 0;
	map_id_ = 0;
	moving_speed_ = 400;
	
	name_ = "Igge" + to_string(getID());
}

void Character::changeMoveStatus(bool moving, double x, double y, int direction) {
	if (moving == moving_) {
		if (moving && direction == direction_)
			return;
			
		if (!moving)
			return;
	}
	
	moving_ = moving;
	x_ = x;
	y_ = y;
	direction_ = direction;
	
	if (moving)
		started_moving_.start();
}

void Character::setPosition(double x, double y) {
	x_ = x;
	y_ = y;
}

void Character::move() {
	if (!moving_)
		return;
		
	auto time_elapsed = started_moving_.restart();
	double pixels = moving_speed_ * time_elapsed;
	
	double x = x_;
	double y = y_;
	
	switch (direction_) {
		case PLAYER_MOVE_UP: y -= pixels;
			break;
			
		case PLAYER_MOVE_DOWN: y += pixels;
			break;
			
		case PLAYER_MOVE_LEFT: x -= pixels;
			break;
			
		case PLAYER_MOVE_RIGHT: x += pixels;
			break;
	}
	
	setPosition(x, y);
}

bool Character::isMoving() const {
	return moving_;
}

double Character::getX() const {
	return x_;
}

double Character::getY() const {
	return y_;
}

int Character::getMovingDirection() const {
	return direction_;
}

size_t Character::getID() const {
	return id_;
}

int Character::getTextureID() const {
	return texture_id_;
}

int Character::getMapID() const {
	return map_id_;
}

const string& Character::getName() const {
	return name_;
}

double Character::getMovingSpeed() const {
	return moving_speed_;
}