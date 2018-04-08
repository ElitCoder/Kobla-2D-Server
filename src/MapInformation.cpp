#include "MapInformation.h"

MapInformation::MapInformation() :
	map_loader_("client_data/maps") {}

bool MapInformation::isCollision(const Character* character) {
	sf::FloatRect bound(character->getX() - 200, character->getY() - 200, 500, 500);
	
	const auto& layers = map_loader_.getLayers();
	
	for (const auto& layer : layers) {
		if (layer.name != "Collision")
			continue;
			
		for(const auto& object : layer.objects) {
			if (object.getAABB().intersects(bound))
				return true;
        }
	}
	
	return false;
}