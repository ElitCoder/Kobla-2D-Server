#include "ClientData.h"
#include "Log.h"
#include "Config.h"

using namespace std;

/*
	MapData
*/

ClientData::MapData::MapData(int id) :
	map_loader_("client_data/maps") {
	id_ = id;
}

int ClientData::MapData::getID() const {
	return id_;
}

bool ClientData::MapData::isCollision(double x, double y, double width, double height) {
	sf::FloatRect bound(x, y, width, height);
	
	// Is the bound outside of the map?
	if (x < 0 || y < 0)
		return true;
		
	if (x + width > map_loader_.getMapSize().x || y + height > map_loader_.getMapSize().y)
		return true;
	
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

void ClientData::MapData::load(const string& filename) {
	if (!map_loader_.load(filename))
		Log(WARNING) << "Map " << filename << " could not be loaded\n";
}

/*
	ClientData
*/

ClientData::ClientData() {}

ClientData::~ClientData() {
	for_each(textures_.begin(), textures_.end(), [] (auto& peer) { delete peer.second; });
}

shared_ptr<ClientData::MapData>& ClientData::getMapData(int map_id) {
	auto iterator = find_if(maps_.begin(), maps_.end(), [&map_id] (auto& map) {
		return map_id == map->getID();
	});
	
	if (iterator == maps_.end()) {
		maps_.push_back(shared_ptr<MapData>(new MapData(map_id)));
		
		auto& map_data = maps_.back();
		map_data->load(getMapName(map_id));
		
		return map_data;
	} else {
		return *iterator;
	}
}

bool ClientData::isMovePossible(const Character* character, double distance, int direction) {
	auto& map_data = getMapData(character->getMapID());
	auto* texture = getTexture(getTexturePath() + getTextureName(character->getTextureID()));
	
	double x = character->getX();
	double y = character->getY();
	
	switch (direction) {
		case PLAYER_MOVE_DOWN: return !map_data->isCollision(x, y, texture->getSize().x, texture->getSize().y + distance);
			break;
			
		case PLAYER_MOVE_LEFT: return !map_data->isCollision(x - distance, y, distance + texture->getSize().x, texture->getSize().y);
			break;
			
		case PLAYER_MOVE_RIGHT: return !map_data->isCollision(x, y, distance + texture->getSize().x, texture->getSize().y);
			break;
			
		case PLAYER_MOVE_UP: return !map_data->isCollision(x, y - distance, texture->getSize().x, texture->getSize().y + distance);
			break;
	}
	
	Log(WARNING) << "Logic should not get here, isMovePossible()\n";
	
	return false;
}

sf::Texture* ClientData::getTexture(const string& filename) {
	auto iterator = find_if(textures_.begin(), textures_.end(), [&filename] (auto& peer) { return peer.first == filename; });
	
	if (iterator == textures_.end()) {
		sf::Texture* texture = new sf::Texture;
		
		if (!texture->loadFromFile(filename))
			Log(WARNING) << "Could not load texture " << filename << endl;
			
		textures_.push_back({ filename, texture });	
		return texture;
	} else {
		return iterator->second;
	}
}

static void loadDataID(vector<pair<int, string>>& container, const string& path) {
	if (!container.empty())
		return;
		
	Config config;
	config.parse(path);
	
	for (auto& peer : config.internal()) {
		//Log(DEBUG) << peer.first << endl;
		//Log(DEBUG) << peer.second.front() << endl;
		
		container.push_back({ stoi(peer.first), peer.second.front() });
	}
}

static string findDataId(vector<pair<int, string>>& container, int id) {
	auto iterator = find_if(container.begin(), container.end(), [id] (auto& peer) { return peer.first == id; });
	
	if (iterator == container.end()) {
		Log(WARNING) << "Can't find ID from parsing config\n";
		
		return "";
	}
	
	return (*iterator).second;
}

string ClientData::getMapName(int id) {
	loadDataID(map_names_, "client_data/maps/id");
	
	return findDataId(map_names_, id);
}

string ClientData::getTextureName(int id) {
	loadDataID(texture_names_, "client_data/textures/id");
	
	return findDataId(texture_names_, id);
}

string ClientData::getTexturePath() {
	return "client_data/textures/";
}