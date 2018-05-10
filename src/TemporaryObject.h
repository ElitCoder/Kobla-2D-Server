#pragma once
#ifndef TEMPORARY_OBJECT_H
#define TEMPORARY_OBJECT_H

#include "Object.h"

enum {
	TEMP_OBJECT_BULLET = 4
};

/*
	Bullets etc
*/
class TemporaryObject : public Object {
public:
	TemporaryObject();
	
	void setType(int type);
	int getType() const;
	
	void setOwner(int id);
	int getOwner() const;
	
private:
	int owner_	= -1;
};

#endif