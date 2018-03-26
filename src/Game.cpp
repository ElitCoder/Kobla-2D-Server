#include "Game.h"
#include "Log.h"
#include "Base.h"
#include "PacketCreator.h"

#include <algorithm>

using namespace std;

Game::Game() {
}

Player* Game::getPlayer(const Connection& connection) {
	auto iterator = find_if(players_.begin(), players_.end(), [&connection] (auto& player) {
		return player.getConnectionID() == connection.getSocket();
	});
	
	if (iterator == players_.end())
		return nullptr;
		
	return &(*iterator);
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
	auto answer = PacketCreator::spawn();
	
	Base::network().send(current_connection_, answer);
}

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
			
		default: {
			Log(NETWORK) << "Unknown packet header " << header << endl;
			handleUnknownPacket();
		}
	}
	
	current_player_ = nullptr;
	current_connection_ = nullptr;
	current_packet_ = &packet;
}

void Game::logic() {
}