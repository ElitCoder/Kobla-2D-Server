#pragma once
#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {
public:
	Timer();
	
	void start();
	void start(size_t ms);
	
	double restart();
	bool elapsed();
	
private:
	std::chrono::time_point<std::chrono::system_clock> start_time_;
	std::chrono::time_point<std::chrono::system_clock> elapsed_time_;
};

#endif