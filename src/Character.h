#ifndef CHARACTER_H
#define CHARACTER_H

#include "Object.h"

enum {
	CHARACTER_CLOSE_DISTANCE = 50,
	CHARACTER_CASUAL_STROLLING_DISTANCE = 150,
	CHARACTER_CASUAL_STROLLING_WAITING_MIN_MS = 100,
	CHARACTER_CASUAL_STROLLING_WAITING_MAX_MS = 2000,
};

class Timer;

class Character : public Object {
public:
	double getCurrentHealth() const;
	double getFullHealth() const;
	
protected:
	Character();
	
	double full_health_;
	double current_health_;
};

#endif