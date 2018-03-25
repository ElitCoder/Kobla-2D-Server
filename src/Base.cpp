#include "Base.h"

Config Base::settings_;
NetworkCommunication Base::network_;

Config& Base::settings() {
	return settings_;
}

NetworkCommunication& Base::network() {
	return network_;
}