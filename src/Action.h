#pragma once
#ifndef ACTION_H
#define ACTION_H

#include <string>
#include <vector>

class Object;

class Action {
public:	
	void activate(Object* object, Object* activater) const;
	
	void setID(int id);
	int getID() const;
	
	void addText(const std::string& text);
	
private:
	int id_ = -1;
	
	std::vector<std::string> texts_;
};

#endif