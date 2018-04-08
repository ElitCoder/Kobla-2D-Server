#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H

#include "Character.h"

// Needed for map information
#include <tmx/MapLoader.hpp>

class ClientData {
public:
	//bool isCollision(const Character* character);
	
private:
	class MapData {
	public:
		MapData();
		
	private:	
		tmx::MapLoader map_loader_;
		
		// Maybe not needed
		size_t id_;
	};
	
	std::vector<MapData> maps_;
};

#endif