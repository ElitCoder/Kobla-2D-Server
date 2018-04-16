#include "Object.h"

using namespace std;

extern size_t g_character_id;

void Object::setValidID() {
	id_ = g_character_id++;
}

void Object::setCollision(bool collision) {
	collision_ = collision;
}

bool Object::getCollision() const {
	return collision_;
}

void Object::changeMoveStatus(bool moving, double x, double y, int direction) {
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

void Object::setPosition(double x, double y) {
	x_ = x;
	y_ = y;
}

void Object::move() {
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

bool Object::isMoving() const {
	return moving_;
}

double Object::getX() const {
	return x_;
}

double Object::getY() const {
	return y_;
}

int Object::getMovingDirection() const {
	return direction_;
}

size_t Object::getID() const {
	return id_;
}

int Object::getTextureID() const {
	return texture_id_;
}

int Object::getMapID() const {
	return map_id_;
}

const string& Object::getName() const {
	return name_;
}

double Object::getMovingSpeed() const {
	return moving_speed_;
}

void Object::setName(const string& name) {
	name_ = name;
}

void Object::setTextureID(int texture_id) {
	texture_id_ = texture_id;
}

void Object::setMapID(int map_id) {
	map_id_ = map_id;
}

double Object::getDistanceMoved() const {
	return distance_moved_;
}

void Object::setPredeterminedDistance(double distance) {
	predetermined_distance_ = distance;
}

double Object::getPredeterminedDistance() const {
	return predetermined_distance_;
}