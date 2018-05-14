#pragma once
#ifndef CHARACTER_H
#define CHARACTER_H

#include "Object.h"

enum {
	CHARACTER_CLOSE_DISTANCE = 50,
	CHARACTER_CASUAL_STROLLING_DISTANCE = 150,
};

class Timer;

class Character : public Object {
public:
	double getCurrentHealth() const;
	double getFullHealth() const;
	
	void setColliding(bool status);
	bool isColliding() const;
	
protected:
	Character();
	
	double full_health_;
	double current_health_;
	
	bool colliding_	= false;
};

#endif