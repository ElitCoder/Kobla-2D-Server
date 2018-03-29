#include "Game.h"
#include "Log.h"
#include "Base.h"
#include "PacketCreator.h"

#include <algorithm>
#include <fstream>

using namespace std;

extern mutex g_main_sync;

Game::Game() {}

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
}

static void loadDataID(vector<pair<int, deque<string>>>& container, const string& path) {
	if (!container.empty())
		return;
		
	Config config;
	config.parse(path);
	
	for (auto& peer : config.internal())
		container.push_back({ stoi(peer.first), peer.second });
}

/*
static deque<string> findDataId(vector<pair<int, deque<string>>>& container, int id) {
	auto iterator = find_if(container.begin(), container.end(), [id] (auto& peer) { return peer.first == id; });
	
	if (iterator == container.end()) {
		Log(WARNING) << "Can't find ID from parsing config\n";
		
		return deque<string>();
	}
	
	return (*iterator).second;
}
*/

void Game::parseNPCs(const vector<pair<int, deque<string>>>& npcs) {
	for (auto& peer : npcs) {
		auto id = peer.first;
		auto name = peer.second.front();
		
		Config config;
		config.parse("data/npcs/" + name);
		
		auto real_name = config.get<string>("name");
		auto texture_id = config.get<int>("texture_id");
		
		NPC npc;
		npc.setNPCID(id);
		npc.setName(real_name);
		npc.setTextureID(texture_id);
		
		reference_npcs_.push_back(npc);
	}
}

NPC* Game::getReferenceNPC(int id) {
	auto iterator = find_if(reference_npcs_.begin(), reference_npcs_.end(), [&id] (auto& npc) { return npc.getNPCID() == (unsigned int)id; });
	
	if (iterator == reference_npcs_.end()) {
		Log(WARNING) << "Could not find reference NPC with ID " << id << endl;
		
		return nullptr;
	}
		
	return &*iterator;
}

static deque<string> getTokens(string input, char delimiter) {
	istringstream stream(input);
	deque<string> tokens;
	string token;
	
	while (getline(stream, token, delimiter))
		if (!token.empty())
			tokens.push_back(token);
	
	return tokens;
}

// TODO: Make this better later on
void Game::parseMaps(const vector<pair<int, deque<string>>>& maps) {
	for (auto& peer : maps) {
		auto id = peer.first;
		auto name = peer.second.front();
		
		ifstream file("data/maps/" + name);
		
		if (!file.is_open())
			Log(WARNING) << "Could not open Map file " << name << endl;
			
		string tmp;
		
		while (getline(file, tmp)) {
			if (tmp.empty() || tmp.front() == '#')
				continue;

			auto tokens = getTokens(tmp, ' ');
			
			// Remove ':'
			tokens.front().pop_back();
			
			// Add NPC
			if (tokens.front() == "npc") {
				auto npc_id = stoi(tokens.at(1));
				auto x = stoi(tokens.at(2));
				auto y = stoi(tokens.at(3));
				
				NPC npc = *getReferenceNPC(npc_id);
				npc.setPosition(x, y);
				npc.setMapID(id);
				// Otherwise the NPC will have the same ID as the reference
				npc.setValidID();
				
				npcs_.push_back(npc);
				
				Log(DEBUG) << "Added NPC " << npc.getName() << " on map " << id << endl;
			}
		}
	}
}

void Game::load() {
	// Load NPCs
	vector<pair<int, deque<string>>> saved_npcs;
	loadDataID(saved_npcs, "data/npcs/id");
	parseNPCs(saved_npcs);
	
	// Load maps
	vector<pair<int, deque<string>>> saved_maps;
	loadDataID(saved_maps, "data/maps/id");
	parseMaps(saved_maps);
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

vector<Player*> Game::getPlayersOnMap(const vector<Player*>& except, int map_id) {
	vector<Player*> players;
	
	for (auto& player : players_) {
		if (player.getMapID() != map_id)
			continue;
			
		// The player exists in except
		if (find_if(except.begin(), except.end(), [&player] (auto& other) {
			return player.getID() == other->getID();
		}) != except.end())
			continue;
			
		players.push_back(&player);	
	}
	
	return players;
}

vector<NPC*> Game::getNPCsOnMap(int map_id) {
	vector<NPC*> npcs;
	
	for (auto& npc : npcs_) {
		if (npc.getMapID() != map_id)
			continue;
			
		npcs.push_back(&npc);	
	}
	
	return npcs;
}

void Game::handleLogin() {
	auto username = current_packet_->getString();
	auto password = current_packet_->getString();
	
	Log(GAME) << "Login from " << username << ":" << password << endl;
	
	auto success = Base::database().checkLogin(username, password);
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
	
	Log(DEBUG) << "Player got connection ID " << current_connection_->getUniqueID() << endl;
	
	auto answer = PacketCreator::spawn(player);
	Base::network().send(current_connection_, answer);
	
	auto other = PacketCreator::addPlayer(&player);
	Base::network().sendToAllExcept(other, { current_connection_->getSocket() });
	
	// Add the already spawned players
	auto players = getPlayersOnMap({ &player }, player.getMapID());
	
	for_each(players.begin(), players.end(), [this] (auto* other) {
		auto add_player_packet = PacketCreator::addPlayer(other);
		Base::network().send(current_connection_, add_player_packet);
	});
	
	auto npcs = getNPCsOnMap(player.getMapID());
	
	for_each(npcs.begin(), npcs.end(), [this] (auto* other) {
		auto add_player_packet = PacketCreator::addPlayer(other);
		Base::network().send(current_connection_, add_player_packet);
	});
}

void Game::handleMove() {
	if (current_player_ == nullptr)
		Log(WARNING) << "Player is nullptr in moving\n";
		
	auto moving = current_packet_->getBool();
	auto x = current_packet_->getFloat();
	auto y = current_packet_->getFloat();
	auto direction = current_packet_->getInt();
	
	current_player_->changeMoveStatus(moving, x, y, direction);
	auto answer = PacketCreator::move(current_player_);
	
	// Send to all except the moving player
	Base::network().sendToAllExcept(answer, { current_connection_->getSocket() });
}