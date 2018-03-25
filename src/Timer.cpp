#include "Timer.h"
#include "Handle.h"

void Timer::start(bool fast)
{
    started = true;
    startTicks = getTime(fast);
}

int Timer::get_ticks(bool fast)
{
    if(started == true)
    {
		int times = getTime(fast);

		if((times - startTicks) <= 0)
			return 0;
		
		else
			return (times - startTicks);
    }

    return 0;
}