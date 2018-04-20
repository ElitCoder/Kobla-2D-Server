#include "TemporaryObject.h"

void TemporaryObject::setType(int type) {
	object_type_ = type;
}

// Check for collision with monsters, outside of map etc.
void TemporaryObject::react() {
}