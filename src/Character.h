#pragma once
#ifndef CHARACTER_H
#define CHARACTER_H

#include "Object.h"

enum {
	CHARACTER_CLOSE_DISTANCE = 75,
	CHARACTER_CASUAL_STROLLING_DISTANCE = 200,
};

class Timer;

class Character : public Object {
public:
	double getCurrentHealth() const;
	double getFullHealth() const;
	
	void setCurrentHealth(double health);
	void setFullHealth(double health, bool heal);
	
	bool reduceHealth(double amount);
	
	void setColliding(bool status);
	bool isColliding() const;
	
	void setAttackSpeed(int ms);
	bool canAttack();
	
protected:
	Character();
	
	double full_health_;
	double current_health_;
	
	bool colliding_	= false;
	
	Timer attack_timer_;
	int attack_speed_ms_ = 0;
};

#endif