#pragma once
#ifndef ACTION_H
#define ACTION_H

#include <string>
#include <vector>

#define TEXT_DISAPPEAR_MS	(10000)

class Object;

class Action {
public:	
	void activate(Object* object, Object* activater) const;
	
	void setID(int id);
	int getID() const;
	
	void setTextResponse(bool response_type);
	void addText(const std::string& text);
	
private:
	int id_				= -1;
	bool response_type_	= false;
	
	std::vector<std::string> texts_;
};

#endif