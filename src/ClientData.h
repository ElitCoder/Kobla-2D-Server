#pragma once
#ifndef CLIENT_DATA_H
#define CLIENT_DATA_H

// Needed for map information
#include <tmx/MapLoader.hpp>

#include <string>
#include <memory>
#include <vector>

class Object;
class ObjectInformation;
class Character;

class ClientData {
public:
	ClientData();
	~ClientData();
	
	ClientData(const ClientData& client) = delete;
	ClientData(const ClientData&& client) = delete;
	
	bool operator=(const ClientData& client) = delete;
	bool operator=(const ClientData&& client) = delete;
	
	bool isMovePossible(const Character* character, double distance, int direction);
	bool isCollision(Object* object, double x, double y);
	bool isCollision(const sf::FloatRect& box, const Object* target);
	
	std::pair<double, double> getBulletPosition(const Object* shooter);
	
	// Taken from Client
	sf::Texture* getTexture(int id);
	
	void runWarm();
	
private:
	class MapData {
	public:
		MapData(int id);
		
		void load(const std::string& filename);
		
		int getID() const;
		bool isCollision(double x, double y, double width, double height, bool collision = true, bool has_constraint = false, const sf::FloatRect& constraint = sf::FloatRect());
		
	private:	
		tmx::MapLoader map_loader_;
		
		int id_;
	};
	
	// Taken from Client
	ObjectInformation& getObjectInformation(int id);
	
	std::string getMapName(int id);
	std::string getTextureName(int id);
	std::string getObjectInformationName(int id);
	
	std::string getMapPath();
	std::string getTexturePath();
	std::string getObjectInformationPath();
	
	sf::FloatRect getScaledCollisionBoxSize(const Object* object, double x, double y, bool only_boots);
	
	std::shared_ptr<MapData>& getMapData(int map_id);
	
	std::vector<std::shared_ptr<MapData>> maps_;
	
	std::vector<std::pair<std::string, sf::Texture*>> textures_;
	std::vector<std::pair<std::string, ObjectInformation>> objects_;
	
	std::vector<std::pair<int, std::string>> map_names_;
	std::vector<std::pair<int, std::string>> texture_names_;
	std::vector<std::pair<int, std::string>> object_names_;
};

#endif