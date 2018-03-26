#include "Game.h"
#include "Log.h"

using namespace std;

Game::Game() {
}

void Game::process(Connection& connection, Packet& packet) {
	auto header = packet.getByte();
	
	switch(header) {
		default: Log(NETWORK) << "Unknown packet header " << header << endl;
	}
}

void Game::logic() {
}