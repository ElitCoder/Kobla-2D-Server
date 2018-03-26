#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <iostream>
#include <sstream>

#define PRINT_STREAM	(std::cout)

enum {
	DEBUG,
	INFORMATION,
	NONE,
	ERROR,
	WARNING
};

class Log : public std::ostringstream {
public:
	Log();
	Log(int level);
	
	~Log();

private:
	static std::mutex print_mutex_;
	int level_;
};

#endif