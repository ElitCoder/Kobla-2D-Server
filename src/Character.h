#ifndef CHARACTER_H
#define CHARACTER_H

#include "Timer.h"

#include <string>

enum {
	PLAYER_MOVE_RIGHT,
	PLAYER_MOVE_DOWN,
	PLAYER_MOVE_LEFT,
	PLAYER_MOVE_UP,
	PLAYER_MOVE_MAX
};

class Character {
public:
	void changeMoveStatus(bool moving, double x, double y, int direction);
	void move();
	
	void setPosition(double x, double y);
	void setName(const std::string& name);
	void setTextureID(int texture_id);
	void setMapID(int map_id);
	void setCollision(bool collision);
	
	bool isMoving() const;
	double getX() const;
	double getY() const;
	int getMovingDirection() const;
	size_t getID() const;
	int getTextureID() const;
	int getMapID() const;
	const std::string& getName() const;
	double getMovingSpeed() const;
	bool getCollision() const;
	double getCurrentHealth() const;
	double getFullHealth() const;
	double getDistanceMoved() const;
	
	void setValidID();
	
protected:
	Character();
	
	std::string name_;
	size_t id_;
	
	Timer started_moving_;
	int direction_			= -1;
	bool moving_			= false;
	double moving_speed_;
	bool collision_			= false;
	double distance_moved_	= 0;
	
	double x_;
	double y_;
	
	int texture_id_;
	int map_id_;
	
	double full_health_;
	double current_health_;
};

#endif