#ifndef OBJECT_H
#define OBJECT_H

enum {
	OBJECT_TYPE_BULLET
};

class Object {
public:
	Object(int type);
	
	void move();
	
	void setMovementDirection(int direction);
	
private:
	int type_		= -1;
	int direction_	= -1;
};

#endif