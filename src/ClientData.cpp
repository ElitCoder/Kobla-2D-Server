#include "ClientData.h"
#include "Log.h"
#include "Object.h"
#include "Character.h"
#include "Base.h"
#include "Game.h"

using namespace std;

/*
	MapData
*/

ClientData::MapData::MapData(int id) :
	map_loader_("client_data/maps")
{
	id_ = id;
}

int ClientData::MapData::getID() const {
	return id_;
}

bool ClientData::MapData::isCollision(double x, double y, double width, double height, bool collision, bool has_constraint, const sf::FloatRect& constraint) {
	sf::FloatRect bound(x, y, width, height);
	
	// Check extra constraint first
	if (has_constraint)
		if (x < constraint.left || x + width > constraint.left + constraint.width || y < constraint.top || y + height > constraint.top + constraint.height)
			return true;
	
	// Is the bound outside of the map?
	if (x < 0 || y < 0)
		return true;
		
	if (x + width > map_loader_.getMapSize().x || y + height > map_loader_.getMapSize().y)
		return true;
	
	// Don't check for map collisions if the Object does not collide with the map
	if (!collision)
		return false;

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

sf::FloatRect ClientData::getScaledCollisionBoxSize(const Object* object, double x, double y, bool only_boots) {
	// Calculate the actual collision detection box, since using the image full size can be too hard on the map
	auto total_size = getObjectInformation(object->getObjectID()).getSize();
	
	sf::FloatRect box;
	box.top = y;
	box.left = x;
	box.width = total_size.front();
	box.height = total_size.back();
	
	auto collision_scale = getObjectInformation(object->getObjectID()).getCollisionScale();
	auto size_x = box.width * collision_scale.front() * (only_boots ? 0.5 : 1.0);
	auto size_y = box.height * collision_scale.back();
	
	box.top += box.height / 2 - size_y / 2;
	box.left += box.width / 2 - size_x / 2;
	box.width = size_x;
	box.height = size_y;
	
	// Only collision check with the boots
	if (only_boots) {
		auto body = box.height * 0.7;
		auto boots = box.width * 0.8;
		
		box.top += body;
		box.height -= body;
		box.left += box.width / 2 - boots / 2;
		box.width = boots;
	}
	
	return box;
}

bool ClientData::isMovePossible(const Character* character, double distance, int direction) {
	auto& map_data = getMapData(character->getMapID());
	auto object_size = getObjectInformation(character->getObjectID()).getSize();
	
	double x = character->getX();
	double y = character->getY();
	
	bool is_monster = character->getObjectType() == OBJECT_TYPE_MONSTER;
	sf::FloatRect constraint;
	
	if (is_monster) {
		// We know it's a Monster
		Monster* monster = (Monster*)character;
		
		constraint.top = monster->getSpawnPoint().getFromY();
		constraint.left = monster->getSpawnPoint().getFromX();
		constraint.width = monster->getSpawnPoint().getToX() - constraint.left + object_size.front();
		constraint.height = monster->getSpawnPoint().getToY() - constraint.top + object_size.back();
	}
	
	switch (direction) {
		case PLAYER_MOVE_DOWN:	return !map_data->isCollision(x, y, object_size.front(), object_size.back() + distance, true, is_monster, constraint);
		case PLAYER_MOVE_LEFT:	return !map_data->isCollision(x - distance, y, distance + object_size.front(), object_size.back(), true, is_monster, constraint);
		case PLAYER_MOVE_RIGHT: return !map_data->isCollision(x, y, distance + object_size.front(), object_size.back(), true, is_monster, constraint);
		case PLAYER_MOVE_UP:	return !map_data->isCollision(x, y - distance, object_size.front(), object_size.back() + distance, true, is_monster, constraint);
	}
	
	Log(WARNING) << "Logic should not get here, isMovePossible()\n";
	
	return false;
}

bool ClientData::isCollision(Object* object, double x, double y) {
	auto& map_data = getMapData(object->getMapID());
	auto box = getScaledCollisionBoxSize(object, x, y, true);
	
	if (map_data->isCollision(box.left, box.top, box.width, box.height, object->getCollision(COLLISION_MAP))) {
		object->getCollisionInformation().setID(-1);
		object->getCollisionInformation().setType(COLLISION_MAP);
		
		return true;
	}
		
	// Check for other players, monsters and NPCs
	if (Base::game().isCollision(box, object))
		return true;
		
	return false;
}

// Calculates where the bullet should be shot from depending on direction, etc
pair<double, double> ClientData::getBulletPosition(const Object* shooter) {
	double x = 0;
	double y = 0;
	
	auto object = getObjectInformation(shooter->getObjectID()).getSize();
	auto bullet = getObjectInformation(TEMP_OBJECT_BULLET).getSize();
	
	switch (shooter->getMovingDirection()) {
		case PLAYER_MOVE_DOWN: {
			x = object.front() / 2 - bullet.front() / 2;
			y = object.back();
			
			break;
		}
		
		case PLAYER_MOVE_LEFT: {
			x = -bullet.front();
			y = object.back() / 2 - bullet.back() / 2;
			
			break;
		}
		
		case PLAYER_MOVE_RIGHT: {
			x = object.front();
			y = object.back() / 2 - bullet.back() / 2;
			
			break;
		}
		
		case PLAYER_MOVE_UP: {
			x = object.front() / 2 - bullet.front() / 2;
			y = -bullet.back();
			
			break;
		}
		
		default: Log(WARNING) << "Bullet position with direction < 0\n";
	}
	
	x += shooter->getX();
	y += shooter->getY();
	
	return { x, y };
}

bool ClientData::isCollision(const sf::FloatRect& box, const Object* object) {
	auto object_size = getObjectInformation(object->getObjectID()).getSize();
	
	return box.intersects(sf::FloatRect(object->getX(), object->getY(), object_size.front(), object_size.back()));
}

sf::Texture* ClientData::getTexture(int id) {
	auto filename = getTextureName(id);
	auto iterator = find_if(textures_.begin(), textures_.end(), [&filename] (auto& peer) { return peer.first == filename; });
	
	if (iterator == textures_.end()) {
		sf::Texture* texture = new sf::Texture;
		
		if (!texture->loadFromFile(getTexturePath() + filename))
			Log(WARNING) << "Could not load texture " << filename << endl;
			
		textures_.push_back({ filename, texture });	
		return texture;
	} else {
		return iterator->second;
	}
}

ObjectInformation& ClientData::getObjectInformation(int id) {
	auto filename = getObjectInformationName(id);
	auto iterator = find_if(objects_.begin(), objects_.end(), [&filename] (auto& peer) { return peer.first == filename; });
	
	if (iterator == objects_.end()) {
		Config config;
		config.parse(getObjectInformationPath() + filename);
		
		ObjectInformation information;
		information.setConfig(config);
		
		objects_.push_back({ filename, information });
		return objects_.back().second;
	} else {
		return iterator->second;
	}
}

static void loadDataID(vector<pair<int, string>>& container, const string& path) {
	if (!container.empty())
		return;
		
	Config config;
	config.parse(path);
	
	for (auto& peer : config.internal()) 
		container.push_back({ stoi(peer.first), peer.second.back() });
}

static string findDataId(vector<pair<int, string>>& container, int id) {
	auto iterator = find_if(container.begin(), container.end(), [id] (auto& peer) { return peer.first == id; });
	
	if (iterator == container.end()) {
		Log(WARNING) << "Can't find ID " << id << " from parsing config\n";
		
		return "";
	}
	
	return (*iterator).second;
}

void ClientData::runWarm() {
	Log(DEBUG) << "Preloading everything to avoid hiccups\n";
	
	// Load up vectors
	getTextureName(0);
	getMapName(0);
	getObjectInformationName(0);
	
	Log(INFORMATION) << "Loading MapData\n";
	
	for (auto& peer : map_names_)
		getMapData(peer.first);
	
	Log(INFORMATION) << "Loading Textures\n";
	
	// Load data from disk
	for (auto& peer : texture_names_)
		getTexture(peer.first);
		
	Log(INFORMATION) << "Loading Objects\n";
		
	for (auto& peer : object_names_)
		getObjectInformation(peer.first);
}

string ClientData::getMapName(int id) {
	loadDataID(map_names_, getMapPath() + "/id");
	
	return findDataId(map_names_, id);
}

string ClientData::getTextureName(int id) {
	loadDataID(texture_names_, getTexturePath() + "/id");
	
	return findDataId(texture_names_, id);
}

string ClientData::getObjectInformationName(int id) {
	loadDataID(object_names_, getObjectInformationPath() + "/id");
	
	return findDataId(object_names_, id);
}

string ClientData::getTexturePath() {
	return "client_data/textures/";
}

string ClientData::getObjectInformationPath() {
	return "client_data/objects/";
}

string ClientData::getMapPath() {
	return "client_data/maps/";
}