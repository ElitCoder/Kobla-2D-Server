#ifndef TEMPORARY_OBJECT_H
#define TEMPORARY_OBJECT_H

#include "Object.h"

enum {
	TEMP_OBJECT_BULLET
};

/*
	Bullets etc
*/
class TemporaryObject : public Object {
public:
	void setType(int type);
	
private:
	int object_type_ = -1;	
};

#endif