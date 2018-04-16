#ifndef CHARACTER_H
#define CHARACTER_H

#include "Timer.h"
#include "Object.h"

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