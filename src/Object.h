#pragma once
#ifndef OBJECT_H
#define OBJECT_H

#include "Config.h"
#include "Timer.h"

#include <string>
#include <array>

enum {
	PLAYER_MOVE_RIGHT,
	PLAYER_MOVE_DOWN,
	PLAYER_MOVE_LEFT,
	PLAYER_MOVE_UP,
	PLAYER_MOVE_MAX
};

enum {
	COLLISION_PLAYERS,
	COLLISION_NPCS,
	COLLISION_MONSTERS,
	COLLISION_MAP,
	COLLISION_MAX
};

enum {
	OBJECT_TYPE_PLAYER,
	OBJECT_TYPE_MONSTER,
	OBJECT_TYPE_NPC,
	OBJECT_TYPE_TEMP
};

class ObjectHit {
public:
	void setID(int id);
	void setType(int type);
	
	int getID() const;
	int getType() const;
	
private:
	int by_id_		= -1;
	int by_type_	= -1;
};

class ObjectInformation {
public:
	void setConfig(const Config& config);
	
	double getScale() const;
	std::array<double, 2> getSize() const;
	std::array<double, 2> getCollisionScale() const;
	
private:
	Config config_;

	int texture_id_				= -1;
	double scale_				= 1;
	double collision_scale_x	= 1;
	double collision_scale_y	= 1;
	
	double size_x_				= 0;
	double size_y_				= 0;
};

class Object {
public:
	virtual ~Object();
	
	static int translateObjectTypeToCollision(int type);
	
	bool changeMoveStatus(bool moving, double x, double y, int direction);
	bool move();
	void setDeterminedDestination(int direction, double distance);
	void setDeterminedDestination(const std::pair<double, double>& destination);
	
	void setPosition(double x, double y);
	void setName(const std::string& name);
	void setObjectID(int id);
	void setMapID(int map_id);
	void setCollision(int type, bool collision);
	void setMovingSpeed(double speed);
	void setMovingDirection(int direction);
	void setActions(const std::vector<int>& actions);
	void setAttack(double value);
	void setFollowing(bool status, int id = -1);
	
	bool isMoving() const;
	double getX() const;
	double getY() const;
	int getMovingDirection() const;
	int getID() const;
	int getObjectID() const;
	int getObjectType() const;
	int getMapID() const;
	const std::string& getName() const;
	double getMovingSpeed() const;
	bool getCollision(int type) const;
	const std::array<bool, COLLISION_MAX>& getCollisions() const;
	std::pair<double, double> getDeterminedDestination() const;
	ObjectHit& getCollisionInformation();
	double getAttack() const;
	bool isFollowing() const;
	int getFollowingID() const;
	
	bool hasActions() const;
	bool reachedDeterminedDistance() const;
	bool hasDeterminedDestination() const;
	
	void setValidID();
	
	void activate(Object* activater);
	void attack(Object* target);
	
	void reactToDamage(int attacker_id);
	int wasAttackedBy() const;
	
	// For distance formula
	double getMiddleX() const;
	double getMiddleY() const;
	
protected:
	Object();
	
	std::string name_				= "";
	int id_							= -1;
	
	Timer started_moving_;
	int direction_					= PLAYER_MOVE_RIGHT;
	bool moving_					= false;
	double moving_speed_			= 200;
	double destination_x_			= -1;
	double destination_y_			= -1;
	bool reached_distance_x_		= false;
	bool reached_distance_y_		= false;
	bool determined_destination_	= false;
	bool following_					= false;
	int following_id_				= -1;
	
	int attacked_by_id_				= -1;
	
	// Collision
	std::array<bool, COLLISION_MAX> collisions_;
	
	double x_						= 0;
	double y_						= 0;
	
	// Coordinates before moving, for calculating smooth Monster movement
	double original_x_				= 0;
	double original_y_				= 0;
	
	int object_id_					= -1;
	int map_id_						= -1;
	
	int object_type_				= -1;
	
	// Holds information about what caused the collision
	ObjectHit collision_information_;
	
	// Activation actions
	std::vector<int> actions_;
	
	double attack_ = 1;
};

#endif