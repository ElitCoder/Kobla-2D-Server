#include "Log.h"

using namespace std;

mutex Log::print_mutex_;

Log::Log() {
	level_ = NONE;
}

Log::Log(int level) {
	level_ = level;
}

// TODO: Add functionality for writing to file, etc.
Log::~Log() {
	lock_guard<mutex> guard(print_mutex_);
	
	switch (level_) {
		case NONE:
			break;
			
		case DEBUG: PRINT_STREAM << "[DEBUG]";
			break;
			
		case INFORMATION: PRINT_STREAM << "[INFORMATION]";
			break;
			
		case ERROR: PRINT_STREAM << "[ERROR]";
			break;
			
		default: PRINT_STREAM << "[UNKNOWN ENUM]";
	}
	
	PRINT_STREAM << " " << str();
}