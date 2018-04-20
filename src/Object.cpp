#include "Object.h"
#include "Base.h"
#include "Log.h"

using namespace std;

extern int g_character_id;

/*
	ObjectInformation
*/

void ObjectInformation::setConfig(const Config& config) {
	config_ = config;
	
	texture_id_ = config_.get<int>("texture", -1);
	scale_ = config_.get<double>("scale", 1);
	bool animated = config_.get<bool>("animated", false);
	
	auto* texture = Base::client().getTexture(texture_id_);
	
	// Set size
	if (animated) {
		vector<int> animation_lines;
		
		animation_lines.push_back(config_.get<int>("animation_right", -1));
		animation_lines.push_back(config_.get<int>("animation_down", -1));
		animation_lines.push_back(config_.get<int>("animation_left", -1));
		animation_lines.push_back(config_.get<int>("animation_up", -1));
		
		auto size = texture->getSize().y / animation_lines.size();
		
		size_x_ = size;
		size_y_ = size;
	} else {
		size_x_ = texture->getSize().x;
		size_y_ = texture->getSize().y;
	}
	
	collision_scale_x = config_.get<double>("collision_scale_x", 1);
	collision_scale_y = config_.get<double>("collision_scale_y", 1);
	
	// Multiply with scaling
	size_x_ *= scale_;
	size_y_ *= scale_;
}

array<double, 2> ObjectInformation::getSize() const {
	return {{ size_x_, size_y_ }};
}

double ObjectInformation::getScale() const {
	return scale_;
}

array<double, 2> ObjectInformation::getCollisionScale() const {
	return {{ collision_scale_x, collision_scale_y }};
}

/*
	Object
*/

// It's not possible to make objects of this type
Object::Object() {
	id_ = g_character_id++;
}

Object::~Object() {}

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
	
	// Check collision
	if (Base::client().isCollision(this, x, y))
		return;
	
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

int Object::getID() const {
	return id_;
}

int Object::getObjectID() const {
	return object_id_;
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

void Object::setObjectID(int id) {
	object_id_ = id;
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

void Object::setMovingSpeed(double speed) {
	moving_speed_ = speed;
}

bool Object::isCollidingEverything() const {
	return collision_everything_;
}

void Object::setCollidingEverything(bool status) {
	collision_everything_ = status;
	
	// If we're colliding with everything, set normal collision to true as well
	if (status)
		collision_ = true;
}