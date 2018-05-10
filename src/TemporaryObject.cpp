#include "TemporaryObject.h"

TemporaryObject::TemporaryObject() {
	object_type_ = OBJECT_TYPE_TEMP;
}

void TemporaryObject::setType(int type) {
	setObjectID(type);
	
	switch (type) {
		case TEMP_OBJECT_BULLET: setMovingSpeed(1500);
		break;
	}
	
	// Bullets should hit everything
	setCollision(COLLISION_MAP, false);
	setCollision(COLLISION_MONSTERS, true);
	setCollision(COLLISION_NPCS, true);
	setCollision(COLLISION_PLAYERS, true);
}

int TemporaryObject::getType() const {
	return object_id_;
}

void TemporaryObject::setOwner(int id) {
	owner_ = id;
}

int TemporaryObject::getOwner() const {
	return owner_;
}