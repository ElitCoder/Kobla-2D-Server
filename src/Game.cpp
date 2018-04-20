#include "Game.h"
#include "Log.h"
#include "Base.h"
#include "PacketCreator.h"
#include "Connection.h"
#include "Map.h"
#include "NPC.h"
#include "Monster.h"
#include "Database.h"
#include "NetworkCommunication.h"
#include "Packet.h"
#include "Player.h"
#include "TemporaryObject.h"
#include "ClientData.h"

#include <algorithm>
#include <cmath>

using namespace std;

extern mutex g_main_sync;

void Game::process(Connection& connection, Packet& packet) {
	auto* player = connection.isVerified() ? getPlayer(connection) : nullptr;
	auto header = packet.getByte();
	
	current_player_ = player;
	current_connection_ = &connection;
	current_packet_ = &packet;
	
	switch(header) {
		case HEADER_LOGIN: handleLogin();
			break;
			
		case HEADER_GET_CHARACTERS: handleGetCharacters();
			break;
			
		case HEADER_SPAWN: handleSpawn();
			break;
			
		case HEADER_MOVE: handleMove();
			break;
			
		case HEADER_SHOOT: handleShoot();
			break;
			
		default: {
			Log(NETWORK) << "Unrecognized packet header " << header << endl;
			handleUnknownPacket();
		}
	}
	
	current_player_ = nullptr;
	current_connection_ = nullptr;
	current_packet_ = &packet;
}

void Game::logic() {
	for (auto& player : players_)
		player.move();
		
	for (auto& map : maps_)
		map.react();
}

bool Game::isCollision(const sf::FloatRect& box, const Object* object) {
	// Collide with Players
	auto players = getPlayersOnMap({ object->getID() }, object->getMapID());
	
	for (auto* player : players) {
		if (player->getCollision() || object->isCollidingEverything())
			if (Base::client().isCollision(box, player))
				return true;
	}
	
	// Collide with NPCs
	auto npcs = getNPCsOnMap(object->getMapID());
	
	for (auto& npc : npcs) {
		if (npc.getID() == object->getID())
			continue;
			
		if (npc.getCollision() || object->isCollidingEverything())
			if (Base::client().isCollision(box, &npc))
				return true;
	}
	
	// Collide with Monsters
	auto monsters = getMonstersOnMap(object->getMapID());
	
	for (auto& monster : monsters) {
		if (monster.getID() == object->getID())
			continue;
			
		if (monster.getCollision() || object->isCollidingEverything())
			if (Base::client().isCollision(box, &monster))
				return true;
	}
	
	// TODO: Collide with other TemporaryObjects?
	
	return false;
}

const NPC& Game::getReferenceNPC(int id) const {
	auto iterator = find_if(reference_npcs_.begin(), reference_npcs_.end(), [&id] (auto& npc) {
		return npc.getNPCID() == (unsigned int)id;
	});
	
	if (iterator == reference_npcs_.end())
		Log(ERROR) << "Could not find reference NPC " << id << endl;
		
	return *iterator;	
}

const Monster& Game::getReferenceMonster(int id) const {
	auto iterator = find_if(reference_monsters_.begin(), reference_monsters_.end(), [&id] (auto& monster) {
		return monster.getMonsterID() == id;
	});
	
	if (iterator == reference_monsters_.end())
		Log(ERROR) << "Could not find reference Monster " << id << endl;
		
	return *iterator;	
}

void Game::load() {
	// Load everything from database
	Base::database()->parseNPCs(reference_npcs_);
	Base::database()->parseMonsters(reference_monsters_);
	Base::database()->parseMaps(maps_);
}

void Game::disconnected(const Connection& connection) {
	// Protect game state by main_sync
	lock_guard<mutex> lock(g_main_sync);
	
	// Is the connection even online?
	auto* player = getPlayer(connection);
	
	if (player == nullptr)
		return;
		
	Log(DEBUG) << "Removing player #" << player->getID() << endl;
	
	// Propagate the disconnection status to the clients
	auto packet = PacketCreator::remove(player);
	Base::network().sendToAllExceptUnsafe(packet, { connection.getSocket() });
	
	// Remove in server
	players_.erase(remove_if(players_.begin(), players_.end(), [&connection] (auto& finder) {
		return finder.getConnectionID() == connection.getUniqueID();
	}));
}

void Game::addPlayer(const Player& player) {
	players_.push_back(player);
}

Player* Game::getPlayer(const Connection& connection) {
	auto iterator = find_if(players_.begin(), players_.end(), [&connection] (auto& player) {
		return player.getConnectionID() == connection.getUniqueID();
	});
	
	if (iterator == players_.end())
		return nullptr;
		
	return &(*iterator);
}

vector<Player*> Game::getPlayersOnMap(const vector<int>& except_ids, int map_id) {
	vector<Player*> players;
	
	for (auto& player : players_) {
		if (player.getMapID() != map_id)
			continue;
			
		// The player exists in except
		if (find_if(except_ids.begin(), except_ids.end(), [&player] (auto& id) {
			return (int)player.getID() == id;
		}) != except_ids.end())
			continue;
			
		players.push_back(&player);	
	}
	
	return players;
}

Map& Game::getMap(int map_id) {
	auto iterator = find_if(maps_.begin(), maps_.end(), [&map_id] (auto& map) {
		return map.getID() == map_id;
	});
	
	if (iterator == maps_.end())
		Log(ERROR) << "Could not find map with ID " << map_id << ", something is not getting parsed\n";
		
	return *iterator;	
}

// Just ask the map for the NPCs
vector<NPC>& Game::getNPCsOnMap(int map_id) {
	auto& map = getMap(map_id);
	
	return map.getNPCs();
}

vector<Monster>& Game::getMonstersOnMap(int map_id) {
	return getMap(map_id).getMonsters();
}

static double distanceTo(const Character* from, const Character* to) {
	return sqrt((from->getX() - to->getX()) * (from->getX() - to->getX()) + (from->getY() - to->getY()) * (from->getY() - to->getY()));
}

vector<Monster*> Game::getCloseMonsters(const Character* character) {
	auto& monsters = getMap(character->getMapID()).getMonsters();
	vector<Monster*> closest;
		
	for (auto& monster : monsters) {
		auto distance = distanceTo(&monster, character);
		
		if (distance < CHARACTER_CLOSE_DISTANCE)
			closest.push_back(&monster);
	}
	
	return closest;
}

vector<Player*> Game::getClosePlayers(const Character *character) {
	vector<Player*> closest;
	
	for (auto& player :players_) {
		if (player.getMapID() != character->getMapID())
			continue;
			
		auto distance = distanceTo(&player, character);
		
		if (distance < CHARACTER_CLOSE_DISTANCE)
			closest.push_back(&player);
	}
	
	return closest;
}

void Game::removeMonster(int id) {
	Log(DEBUG) << "Trying to remove monster ID " << id << endl;
	
	for (auto& map : maps_) {
		auto iterator = find_if(map.getMonsters().begin(), map.getMonsters().end(), [&id] (auto& monster) {
			return monster.getID() == id;
		});
		
		if (iterator == map.getMonsters().end())
			continue;
			
		auto packet = PacketCreator::remove(&*iterator);
		Base::network().sendToAllExcept(packet, {});
		
		map.removeMonster(id);
		break;
	}
}

void Game::handleLogin() {
	auto username = current_packet_->getString();
	auto password = current_packet_->getString();
	
	Log(GAME) << "Login from " << username << ":" << password << endl;
	
	auto success = Base::database()->login(username, password);
	auto answer = PacketCreator::answerLogin(success);
	
	Base::network().send(current_connection_, answer);
}

void Game::handleGetCharacters() {
}

void Game::handleUnknownPacket() {
	Base::network().send(current_connection_, PacketCreator::unknown());
}

void Game::handleSpawn() {
	if (current_player_ != nullptr)
		Log(WARNING) << "Spawn packet received, but the connection already holds a player\n";
		
	// Give the connection a temporary player
	Player player;
	player.setConnectionID(current_connection_->getUniqueID());
	addPlayer(player);
	
	// Randomize position since it's not saved
	auto position = getMap(player.getMapID()).getSpawnPoint();
	player.setPosition(position.front(), position.back());
	
	Log(DEBUG) << "Spawning player at " << position.front() << " " << position.back() << endl;
	
	Log(DEBUG) << "Player got connection ID " << current_connection_->getUniqueID() << endl;
	
	auto answer = PacketCreator::spawn(player);
	Base::network().send(current_connection_, answer);
	
	auto other = PacketCreator::addPlayer(&player);
	Base::network().sendToAllExcept(other, { current_connection_->getSocket() });
	
	// Add the already spawned players
	auto players = getPlayersOnMap({ player.getID() }, player.getMapID());
	
	for_each(players.begin(), players.end(), [this] (auto* other) {
		auto add_player_packet = PacketCreator::addPlayer(other);
		Base::network().send(current_connection_, add_player_packet);
	});
	
	// Add NPCs
	auto& npcs = getNPCsOnMap(player.getMapID());
	
	for_each(npcs.begin(), npcs.end(), [this] (auto& other) {
		auto add_player_packet = PacketCreator::addPlayer(&other);
		Base::network().send(current_connection_, add_player_packet);
	});
	
	// Add monsters
	auto& monsters = getMonstersOnMap(player.getMapID());
	
	for_each(monsters.begin(), monsters.end(), [this] (auto& other) {
		auto add_player_packet = PacketCreator::addPlayer(&other);
		Base::network().send(current_connection_, add_player_packet);
	});
}

void Game::updateMovement(Character* character, const vector<int>& sockets) {
	auto answer = PacketCreator::move(character);
	
	// Send to all except the moving player
	Base::network().sendToAllExcept(answer, sockets);
}

void Game::handleMove() {
	if (current_player_ == nullptr)
		Log(WARNING) << "Player is nullptr in moving\n";
		
	auto moving = current_packet_->getBool();
	auto x = current_packet_->getFloat();
	auto y = current_packet_->getFloat();
	auto direction = current_packet_->getInt();
	
	// Player stops moving, don't set the direction to undefined
	if (!moving)
		direction = current_player_->getMovingDirection();
	
	current_player_->changeMoveStatus(moving, x, y, direction);
	updateMovement(current_player_, { current_connection_->getSocket() });
}

void Game::handleShoot() {
	// Shoot something
	// Create a bullet object with direction and speed
	auto& map = getMap(current_player_->getMapID());
	
	TemporaryObject bullet;
	bullet.setType(TEMP_OBJECT_BULLET);
	bullet.setMapID(map.getID());
	bullet.changeMoveStatus(true, current_player_->getX(), current_player_->getY(), current_player_->getMovingDirection());
	
	map.addObject(bullet);
	
	// Propagate the effect to other Clients
	Base::network().sendToAll(PacketCreator::shoot(bullet));
}