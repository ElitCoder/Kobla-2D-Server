#include "Handle.h"

int Random(int lowest, int highest)
{
	return (int)(RandomFloat((float)lowest, (float)highest) + 0.5f);
}

float RandomFloat(float a, float b)
{
	return (((b - a) * ((float)rand() / RAND_MAX)) + a);
}

unsigned int getTime(bool fast)
{
	if(fast)
		return GetTickCount();

	else
	{
		LARGE_INTEGER currentTimer;
		LARGE_INTEGER timePerSecond;

		QueryPerformanceFrequency(&timePerSecond);
		QueryPerformanceCounter(&currentTimer);

		currentTimer.QuadPart /= (timePerSecond.QuadPart / 1000);

		return (unsigned int)currentTimer.QuadPart;
	}
}