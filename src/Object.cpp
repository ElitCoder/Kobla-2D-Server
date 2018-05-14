#include "Object.h"
#include "Base.h"
#include "Log.h"
#include "ClientData.h"
#include "Game.h"

using namespace std;

extern int g_character_id;

/*
	ObjectHit
*/

void ObjectHit::setID(int id) {
	by_id_ = id;
}

void ObjectHit::setType(int type) {
	by_type_ = type;
}

int ObjectHit::getID() const {
	return by_id_;
}

int ObjectHit::getType() const {
	return by_type_;
}

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
	
	collisions_ = array<bool, COLLISION_MAX>{{ false }};
}

Object::~Object() {}

// Static since translation doesn't need an instance
int Object::translateObjectTypeToCollision(int type) {
	switch (type) {
		case OBJECT_TYPE_MONSTER:	return COLLISION_MONSTERS;
		case OBJECT_TYPE_NPC:		return COLLISION_NPCS;
		case OBJECT_TYPE_PLAYER:	return COLLISION_PLAYERS;
	}
	
	Log(WARNING) << "Object type translation " << type << " failed\n";
	
	return -1;
}

void Object::setValidID() {
	id_ = g_character_id++;
}

void Object::setCollision(int type, bool collision) {
	collisions_.at(type) = collision;
}

bool Object::getCollision(int type) const {
	return collisions_.at(type);
}

bool Object::changeMoveStatus(bool moving, double x, double y, int direction) {
	if (moving == moving_) {
		if (moving && direction == direction_)
			return false;
			
		if (!moving)
			return false;
	}
	
	moving_ = moving;
	
	x_ = x;
	y_ = y;
	
	direction_ = direction;
	
	original_x_ = x_;
	original_y_ = y_;
	
	destination_x_ = -1;
	destination_y_ = -1;
	reached_distance_x_ = false;
	reached_distance_y_ = false;
	determined_destination_ = false;
	
	if (moving)
		started_moving_.start();
		
	return true;
}

void Object::setPosition(double x, double y) {
	x_ = x;
	y_ = y;
}

bool Object::move() {
	if (!moving_)
		return false;
		
	auto time_elapsed = started_moving_.restart();
	double pixels = moving_speed_ * time_elapsed;
	
	double x = x_;
	double y = y_;
	
	double diff_x = destination_x_ - x;
	double diff_y = destination_y_ - y;
	
	bool is_reaching_x = false;
	bool is_reaching_y = false;
	
	if (hasDeterminedDestination()) {
		// Move to target
		double diff_x_squared = pow(diff_x, 2);
		double diff_y_squared = pow(diff_y, 2);
		
		double distance = diff_x_squared + diff_y_squared;
		
		double move_x = diff_x_squared / distance * pixels;
		double move_y = diff_y_squared / distance * pixels;
		
		if (move_x >= abs(diff_x))
			is_reaching_x = true;
			
		if (move_y >= abs(diff_y))
			is_reaching_y = true;
		
		if (diff_x < 0)
			x -= move_x;
		else
			x += move_x;
		
		if (diff_y < 0)
			y -= move_y;
		else
			y += move_y;
	} else {
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
	}
	
	// Check collision
	if (Base::client().isCollision(this, x, y))
		return false;
	
	if (hasDeterminedDestination()) {
		if (is_reaching_x)
			reached_distance_x_ = true;
		
		if (is_reaching_y)
			reached_distance_y_ = true;
	}
	
	setPosition(x, y);
	
	return true;
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

void Object::setMovingSpeed(double speed) {
	moving_speed_ = speed;
}

const array<bool, COLLISION_MAX>& Object::getCollisions() const {
	return collisions_;
}

int Object::getObjectType() const {
	return object_type_;
}

ObjectHit& Object::getCollisionInformation() {
	return collision_information_;
}

void Object::setMovingDirection(int direction) {
	direction_ = direction;
}

void Object::activate(Object* activater) {
	Log(DEBUG) << activater->getID() << " trying to activate " << getID() << endl;
	
	// Is the Object able to activate?
	if (!hasActions())
		return;
		
	// Activate first Action for now
	auto& action = Base::game().getReferenceAction(actions_.front());
	
	action.activate(this, activater);
}

void Object::setActions(const vector<int>& actions) {
	actions_ = actions;
}

bool Object::hasActions() const {
	return !actions_.empty();
}

void Object::setDeterminedDestination(int direction, double distance) {
	switch (direction) {
		case PLAYER_MOVE_UP: {
			destination_x_ = x_;
			destination_y_ = y_ - distance;
			
			break;
		}
			
		case PLAYER_MOVE_DOWN: {
			destination_x_ = x_;
			destination_y_ = y_ + distance;
			
			break;
		}
			
		case PLAYER_MOVE_RIGHT: {
			destination_x_ = x_ + distance;
			destination_y_ = y_;
			
			break;
		}
			
		case PLAYER_MOVE_LEFT: {
			destination_x_ = x_ - distance;
			destination_y_ = y_;
			
			break;
		}
			
		default: Log(WARNING) << "Invalid direction " << direction << endl;
	}
	
	reached_distance_x_ = false;
	reached_distance_y_ = false;
	determined_destination_ = true;
}

bool Object::reachedDeterminedDistance() const {
	return reached_distance_x_ && reached_distance_y_;
}

std::pair<double, double> Object::getDeterminedDestination() const {
	return { destination_x_, destination_y_ };
}

bool Object::hasDeterminedDestination() const {
	return determined_destination_;
}

void Object::setDeterminedDestination(const std::pair<double, double>& destination) {
	destination_x_ = destination.first;
	destination_y_ = destination.second;
	
	reached_distance_x_ = false;
	reached_distance_y_ = false;
	determined_destination_ = true;
}