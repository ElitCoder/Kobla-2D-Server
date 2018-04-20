#ifndef OBJECT_H
#define OBJECT_H

#include "Timer.h"
#include "Config.h"

#include <string>

enum {
	PLAYER_MOVE_RIGHT,
	PLAYER_MOVE_DOWN,
	PLAYER_MOVE_LEFT,
	PLAYER_MOVE_UP,
	PLAYER_MOVE_MAX
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
	
	void changeMoveStatus(bool moving, double x, double y, int direction);
	void move();
	void setPredeterminedDistance(double distance);
	
	void setPosition(double x, double y);
	void setName(const std::string& name);
	void setObjectID(int id);
	void setMapID(int map_id);
	void setCollision(bool collision);
	void setMovingSpeed(double speed);
	void setCollidingEverything(bool status);
	
	bool isMoving() const;
	double getX() const;
	double getY() const;
	int getMovingDirection() const;
	int getID() const;
	int getObjectID() const;
	int getMapID() const;
	const std::string& getName() const;
	double getMovingSpeed() const;
	bool getCollision() const;
	double getDistanceMoved() const;
	double getPredeterminedDistance() const;
	bool isCollidingEverything() const;
	
	void setValidID();
	
protected:
	Object();
	
	std::string name_				= "";
	int id_							= -1;
	
	Timer started_moving_;
	int direction_					= PLAYER_MOVE_RIGHT;
	bool moving_					= false;
	double moving_speed_			= 200;
	bool collision_					= false;
	bool collision_everything_		= false;
	double distance_moved_			= 0;
	double predetermined_distance_	= -1;
	
	double x_						= 0;
	double y_						= 0;
	
	// Coordinates before moving, for calculating smooth Monster movement
	double original_x_				= 0;
	double original_y_				= 0;
	
	int object_id_					= -1;
	int map_id_						= -1;
};

#endif