#include "Timer.h"

using namespace std;

Timer::Timer() {
	start();
}

void Timer::start() {
	start_time_ = chrono::system_clock::now();
}

// Get current elapsed time and restart timer
double Timer::restart() {
	auto now = chrono::system_clock::now();
	auto nanoseconds = chrono::duration_cast<chrono::nanoseconds>(now - start_time_);
	start_time_ = now;
	
	// Return seconds
	return nanoseconds.count() / 1e09;
}