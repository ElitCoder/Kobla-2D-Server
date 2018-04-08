#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H

#include "Character.h"

// Needed for map information
#include <tmx/MapLoader.hpp>

class ClientData {
public:
	ClientData();
	~ClientData();
	
	ClientData(const ClientData& client) = delete;
	ClientData(const ClientData&& client) = delete;
	
	bool operator=(const ClientData& client) = delete;
	bool operator=(const ClientData&& client) = delete;
	
	bool isMovePossible(const Character* character, double distance, int direction);
	
private:
	class MapData {
	public:
		MapData(int id);
		
		void load(const std::string& filename);
		
		int getID() const;
		bool isCollision(double x, double y, double width, double height);
		
	private:	
		tmx::MapLoader map_loader_;
		
		int id_;
	};
	
	// Taken from Client
	sf::Texture* getTexture(const std::string& filename);
	std::string getMapName(int id);
	std::string getTextureName(int id);
	std::string getTexturePath();
	
	std::shared_ptr<MapData>& getMapData(int map_id);
	
	std::vector<std::shared_ptr<MapData>> maps_;
	
	std::vector<std::pair<std::string, sf::Texture*>> textures_;
	
	std::vector<std::pair<int, std::string>> map_names_;
	std::vector<std::pair<int, std::string>> texture_names_;
};

#endif