#include "Base.h"

Config Base::settings_;
NetworkCommunication Base::network_;
Game Base::game_;

Config& Base::settings() {
	return settings_;
}

NetworkCommunication& Base::network() {
	return network_;
}

Game& Base::game() {
	return game_;
}