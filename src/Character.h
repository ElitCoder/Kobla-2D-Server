#ifndef CHARACTER_H
#define CHARACTER_H

#include "Timer.h"

#include <string>

enum {
	PLAYER_MOVE_UP,
	PLAYER_MOVE_DOWN,
	PLAYER_MOVE_LEFT,
	PLAYER_MOVE_RIGHT
};

class Character {
public:
	void changeMoveStatus(bool moving, double x, double y, int direction);
	void move();
	
	void setPosition(double x, double y);
	
	bool isMoving() const;
	double getX() const;
	double getY() const;
	int getMovingDirection() const;
	size_t getID() const;
	int getTextureID() const;
	int getMapID() const;
	const std::string& getName() const;
	double getMovingSpeed() const;
	
protected:
	Character();
	
	std::string name_;
	size_t id_;
	
	Timer started_moving_;
	int direction_;
	bool moving_;
	double moving_speed_;
	
	double x_;
	double y_;
	
	int texture_id_;
	int map_id_;
};

#endif