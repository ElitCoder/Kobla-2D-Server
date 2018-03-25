#include "Log.h"

using namespace std;

mutex Log::print_mutex_;

Log::Log() {
	level_ = NONE;
}

Log::Log(int level) {
	level_ = level;
}

Log::~Log() {
	lock_guard<mutex> guard(print_mutex_);
	
	switch (level_) {
		case NONE:
			break;
			
		case DEBUG: cout << "[DEBUG]";
			break;
			
		case INFORMATION: cout << "[INFORMATION]";
			break;
			
		default: cout << "[UNKNOWN ENUM]";
	}
	
	PRINT_STREAM << " " << str();
}