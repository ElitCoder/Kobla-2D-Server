#include "TemporaryObject.h"

void TemporaryObject::setType(int type) {
	object_type_ = type;
	
	setObjectID(type);
	
	switch (type) {
		case TEMP_OBJECT_BULLET: setMovingSpeed(1000);
		break;
	}
	
	// Bullets should hit everything
	setCollidingEverything(true);
}

int TemporaryObject::getType() const {
	return object_type_;
}