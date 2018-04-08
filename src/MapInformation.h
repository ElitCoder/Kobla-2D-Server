#ifndef MAP_INFORMATION_H
#define MAP_INFORMATION_H

#include "Character.h"

// Needed for map information
#include <tmx/MapLoader.hpp>

class MapInformation {
public:
	MapInformation();
	
	bool isCollision(const Character* character);
	
private:
	tmx::MapLoader map_loader_;
};

#endif