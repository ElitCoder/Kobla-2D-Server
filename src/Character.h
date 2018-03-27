#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>

class Character {
public:
	Character();
	
protected:
	std::string name_;
	size_t id_;
};

#endif