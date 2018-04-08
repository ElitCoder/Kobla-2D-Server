#include "Character.h"
#include "Log.h"

using namespace std;

static size_t g_character_id;

Character::Character() {
	id_ = g_character_id++;
	
	Log(DEBUG) << "Running constructor for Character\n";
	
	moving_ = false;
	
	// Hard coded for now
	x_ = 100;
	y_ = 100;
	texture_id_ = 0;
	map_id_ = 0;
	moving_speed_ = 200;
	collision_ = false;
	full_health_ = 100;
	current_health_ = full_health_;
	
	name_ = "Igge" + to_string(getID());
}

void Character::setValidID() {
	id_ = g_character_id++;
}

void Character::setCollision(bool collision) {
	collision_ = collision;
}

bool Character::getCollision() const {
	return collision_;
}

void Character::changeMoveStatus(bool moving, double x, double y, int direction) {
	if (moving == moving_) {
		if (moving && direction == direction_)
			return;
			
		if (!moving)
			return;
	}
	
	moving_ = moving;
	
	if (!moving && predetermined_distance_ > 0) {
		// We had a predetermined distance to move, just set it to that value
		switch (direction_) {
			case PLAYER_MOVE_UP: y_ = original_y_ - predetermined_distance_;
				break;
				
			case PLAYER_MOVE_DOWN: y_ = original_y_ + predetermined_distance_;
				break;
				
			case PLAYER_MOVE_LEFT: x_ = original_x_ - predetermined_distance_;
				break;
				
			case PLAYER_MOVE_RIGHT: x_ = original_x_ + predetermined_distance_;
				break;
		}
	} else {
		x_ = x;
		y_ = y;
	}
	
	direction_ = direction;
	
	original_x_ = x_;
	original_y_ = y_;
	
	predetermined_distance_ = -1;
	
	// Character stopped in this direction, let's reset total distance moved for monsters
	distance_moved_= 0;
	
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
	
	distance_moved_ += pixels;
	
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

void Character::setName(const string& name) {
	name_ = name;
}

void Character::setTextureID(int texture_id) {
	texture_id_ = texture_id;
}

void Character::setMapID(int map_id) {
	map_id_ = map_id;
}

double Character::getCurrentHealth() const {
	return current_health_;
}

double Character::getFullHealth() const {
	return full_health_;
}

double Character::getDistanceMoved() const {
	return distance_moved_;
}

void Character::setPredeterminedDistance(double distance) {
	predetermined_distance_ = distance;
}

double Character::getPredeterminedDistance() const {
	return predetermined_distance_;
}