#ifndef BASE_H
#define BASE_H

#include "Config.h"
#include "NetworkCommunication.h"

class Base {
public:
	static Config& settings();
	static NetworkCommunication& network();
	
private:
	static Config settings_;
	static NetworkCommunication network_;
};

#endif