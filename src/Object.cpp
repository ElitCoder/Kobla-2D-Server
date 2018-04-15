#include "Object.h"

Object::Object(int type) {
	type_ = type;
}

void Object::move() {
}

void Object::setMovementDirection(int direction) {
	direction_ = direction;
}