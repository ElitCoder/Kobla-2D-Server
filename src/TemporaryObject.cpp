#include "TemporaryObject.h"

void TemporaryObject::setType(int type) {
	object_type_ = type;
	
	setObjectID(type);
	
	switch (type) {
		case TEMP_OBJECT_BULLET: setMovingSpeed(400);
		break;
	}
	
	// Bullets should hit everything
	setCollidingEverything(true);
}

int TemporaryObject::getType() const {
	return object_type_;
}

// Check for collision with monsters, outside of map etc.
void TemporaryObject::react() {
}