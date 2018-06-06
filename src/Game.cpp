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

#define FORCED_LOGIC_WAIT_TIME	(50)

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
			
		case HEADER_HIT: handleHit();
			break;
			
		case HEADER_ACTIVATE: handleActivate();
			break;
			
		case HEADER_CHAT: handleChat();
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

bool Game::isCollision(const sf::FloatRect& box, Object* object) {
	if (object->getCollision(COLLISION_PLAYERS)) {
		// Collide with Players
		auto players = getPlayersOnMap({ object->getID() }, object->getMapID());
		
		for (auto* player : players) {
			if (Base::client().isCollision(box, player)) {
				object->getCollisionInformation().setID(player->getID());
				object->getCollisionInformation().setType(COLLISION_PLAYERS);
				
				return true;
			}
		}
	}
	
	// Collide with NPCs
	auto npcs = getNPCsOnMap(object->getMapID());
	
	for (auto& npc : npcs) {
		if (npc.getID() == object->getID())
			continue;
		
		if (object->getCollision(COLLISION_NPCS) || npc.isColliding()) {
			if (Base::client().isCollision(box, &npc)) {
				object->getCollisionInformation().setID(npc.getID());
				object->getCollisionInformation().setType(COLLISION_NPCS);
				
				return true;
			}
		}
	}
	
	if (object->getCollision(COLLISION_MONSTERS)) {
		// Collide with Monsters
		auto monsters = getMonstersOnMap(object->getMapID());
		
		for (auto& monster : monsters) {
			if (monster.getID() == object->getID())
				continue;
			
			if (Base::client().isCollision(box, &monster)) {
				object->getCollisionInformation().setID(monster.getID());
				object->getCollisionInformation().setType(COLLISION_MONSTERS);
				
				return true;
			}
		}
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

const Action& Game::getReferenceAction(int id) const {
	auto iterator = find_if(reference_actions_.begin(), reference_actions_.end(), [&id] (auto& action) { 
		return action.getID() == id;
	});
	
	if (iterator == reference_actions_.end())
		Log(ERROR) << "Could not find reference Action " << id << endl;
		
	return *iterator;
}

void Game::load() {
	// Load everything from database
	Log(INFORMATION) << "Loading NPCs\n";
	Base::database()->parseNPCs(reference_npcs_);
	
	Log(INFORMATION) << "Loading Monsters\n";
	Base::database()->parseMonsters(reference_monsters_);
	
	Log(INFORMATION) << "Loading Maps\n";
	Base::database()->parseMaps(maps_);
	
	Log(INFORMATION) << "Loading Actions\n";
	Base::database()->parseActions(reference_actions_);
	
	// Run warm
	Base::client().runWarm();
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

vector<TemporaryObject>& Game::getObjectsOnMap(int map_id) {
	return getMap(map_id).getTemporaryObjects();
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

vector<Player*> Game::getContactPlayers(const Character* character) {
	vector<Player*> close;
	
	for (auto& player : players_) {
		if (character->getMapID() != player.getMapID())
			continue;
			
		if (Base::client().isCollision(character, &player))
			close.push_back(&player);
	}
	
	return close;
}

vector<Player*> Game::getClosePlayers(const Character *character) {
	vector<Player*> closest;
	
	for (auto& player : players_) {
		if (player.getMapID() != character->getMapID())
			continue;
			
		auto distance = distanceTo(&player, character);
		
		if (distance < CHARACTER_CLOSE_DISTANCE)
			closest.push_back(&player);
	}
	
	return closest;
}

void Game::removeCharacter(int id) {
	for (auto& map : maps_) {
		auto* character = map.getCharacter(id);
		
		if (character == nullptr)
			continue;
			
		auto packet = PacketCreator::remove(character);
		Base::network().sendToAll(packet);
			
		switch (character->getObjectType()) {
			case OBJECT_TYPE_MONSTER: map.removeMonster(id);
				break;
		}
		
		break;
	}
}

// This is just a safeguard that the TemporaryObject actually disappears, compensating for network lagging
void Game::removeObject(const Object* object) {
	auto packet = PacketCreator::remove(object);
	Base::network().sendToAll(packet);
}

void Game::spawnCharacter(const Character* character) {
	auto players = getPlayersOnMap({}, character->getMapID());
	auto packet = PacketCreator::addPlayer(character);
	
	for (auto* player : players)
		Base::network().sendUnique(player->getConnectionID(), packet);
}

Object* Game::getObject(int id) {
	auto player_iterator = find_if(players_.begin(), players_.end(), [&id] (auto& player) { return player.getID() == id; });
	
	if (player_iterator != players_.end())
		return &*player_iterator;
		
	for (auto& map : maps_) {
		auto* object = map.getObject(id);
		
		if (object != nullptr)
			return object;
	}
	
	return nullptr;
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
	
	// Randomize position since it's not saved
	auto position = getMap(player.getMapID()).getSpawnPoint();
	player.setPosition(position.front(), position.back());
	
	Log(DEBUG) << "Spawning player at " << position.front() << " " << position.back() << endl;
	
	Log(DEBUG) << "Player got connection ID " << current_connection_->getUniqueID() << endl;
	
	addPlayer(player);
	
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
	
	// Add TemporaryObjects
	auto& objects = getObjectsOnMap(player.getMapID());
	
	for_each(objects.begin(), objects.end(), [this] (auto& other) {
		auto add_player_packet = PacketCreator::shoot(other);
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
	bullet.setOwner(current_player_->getID());
	bullet.setAttack(current_player_->getAttack());
	
	auto position = Base::client().getBulletPosition(current_player_);
	bullet.changeMoveStatus(true, position.first, position.second, current_player_->getMovingDirection());
	
	map.addObject(bullet);
	
	// Propagate the effect to other Clients
	Base::network().sendToAll(PacketCreator::shoot(bullet));
	
	//Log(DEBUG) << "Player " << current_player_->getID() << " shot bullet at X: " << current_player_->getX() << " Y: " << current_player_->getY() << endl;
}

void Game::handleHit() {
	#if 0
	// Avoid running logic too often
	if (!last_forced_logic_.elapsed())
		return;

	// Trigger logic twice to see if the bullet hits
	// The first updates moves everything and the second update checks for the hit
	logic();
	logic();
	
	last_forced_logic_.start(FORCED_LOGIC_WAIT_TIME);
	#endif
	
	// Instead let the Map decide if the hit is legal
	auto object_id = current_packet_->getInt();
	auto hit_id = current_packet_->getInt();
	
	auto& map = getMap(current_player_->getMapID());
	map.checkHit(current_player_, object_id, hit_id);
}

void Game::handleActivate() {
	// Activate something
	auto id = current_packet_->getInt();
	
	auto* object = getObject(id);
	
	if (object == nullptr)
		return;
	
	object->activate(current_player_);
}

void Game::handleChat() {
	auto message = current_packet_->getString();
	
	// Propagate to everyone
	auto packet = PacketCreator::text(current_player_, message, TEXT_DISAPPEAR_MS);
	Base::network().sendToAll(packet);
	
	Log(DEBUG) << "Got chat message: " << message << endl;
}