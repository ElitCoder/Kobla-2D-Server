#include "Character.h"
#include "Log.h"

Character::Character() {
	static size_t id;
	
	id_ = id++;
	
	Log(DEBUG) << "Running constructor for Character\n";
}