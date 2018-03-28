#include "Game.h"
#include "Log.h"
#include "Base.h"
#include "PacketCreator.h"

#include <algorithm>

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
	
	auto other = PacketCreator::addPlayer(player);
	Base::network().sendToAllExcept(other, { current_connection_->getSocket() });
	
	// Add the already spawned players
	auto players = getPlayersOnMap({ &player }, player.getMapID());
	
	for_each(players.begin(), players.end(), [this] (auto* other) {
		auto add_player_packet = PacketCreator::addPlayer(*other);
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