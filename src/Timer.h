#ifndef TIMER_H
#define TIMER_H

class Timer
{
public:
    Timer()
	{
		this->startTicks = 0;
		this->started = false;
	}

    void start(bool fast);
    int get_ticks(bool fast);

private:
    int startTicks;
	bool started;
};

#endif